#include <Simulation.h>
#include "SPHSolverCPU.h"
#include <random>
#include "math/SmoothedKernel.h"
#include "math/SpikedKernel.h"
#include "util/Util.h"

SPHSolverCPU::SPHSolverCPU(float timeStep, size_t particleCount, const BoxCollider& boxCollider) :
	Solver(timeStep),
	PARTICLE_COUNT(particleCount),
	m_particles(particleCount),
	m_neighborList(particleCount),
	PARTICLE_RADIUS(0.1f),
	KERNEL_RADIUS(PARTICLE_RADIUS*4),
	BOX_COLLIDER(boxCollider)
{
	Setup(Scenario::TwoSided);
}

void SPHSolverCPU::Setup(Scenario scenario)
{
	switch (scenario)
	{
	case Scenario::OneSided:
		setupOneSidedParticles();
		break;
	case Scenario::Fill:
		setupFillParticles();
		break;
	case Scenario::TwoSided:
		setupTwoSidedParticles();
		break;
	default: ;
	}
}

void SPHSolverCPU::Reset(Scenario scenario)
{
	m_particles.Reset();
	Setup(scenario);
}

void SPHSolverCPU::BeginTimeStep()
{
	m_state = ParticleState(m_particles);

	m_tree.Build(reinterpret_cast<std::vector<glm::vec4>&>(m_particles.Positions));
	//LOG_CORE_INFO(glm::to_string(m_state.Positions[0]));

	computeNeighborList();

	computeDensities();
}

void SPHSolverCPU::ApplyForces()
{
	#pragma omp parallel
	{
		#pragma omp for
		for (int i = 0; i < m_particles.Size(); ++i)
			m_particles.Forces[i].vec = GRAVITY;
	}

	viscosityForces();
	pressureForces();
}

void SPHSolverCPU::Integrate()
{
	#pragma omp parallel
	{
		#pragma omp for
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			//integrate velocity
			glm::vec3& newVelocity = m_state.Velocities[i].vec;
			newVelocity = m_particles.Velocities[i].vec + TIMESTEP * m_particles.Forces[i].vec;

			//integrate position
			glm::vec3& newPosition = m_state.Positions[i].vec;
			newPosition = m_particles.Positions[i].vec + TIMESTEP * newVelocity;
		}
	}
}

void SPHSolverCPU::ResolveCollisions()
{
	resolveCollisions(m_state.Positions, m_state.Velocities);
}

void SPHSolverCPU::EndTimeStep()
{
	//move state into particles
	m_particles.Integrate(m_state);

	//fakeViscosity();
}

const ParticleSet& SPHSolverCPU::Particles() const
{
	return m_particles;
}

void SPHSolverCPU::computeNeighborList()
{
	//find all neighbours of all elements
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Positions.size(); ++i)
	{
		std::vector<uint32_t> e;
		e.reserve(1000);
		bool foundE = m_tree.FindNearestNeighbors(m_particles.Positions[i].vec4, KERNEL_RADIUS, e);
		m_neighborList[i] = e;
	}
}

void SPHSolverCPU::computeDensities()
{
	const auto& p = m_particles.Positions;
	auto& d = m_particles.Densities;

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Positions.size(); ++i)
	{
		float sum = sumOfKernelNearby(i);
		d[i] = m_mass * sum;
	}
}

float SPHSolverCPU::sumOfKernelNearby(size_t pointIndex) const
{
	float sum = 0.0f;
	SmoothedKernel kernal = SmoothedKernel(KERNEL_RADIUS);

	for(const auto& neighbor : m_neighborList[pointIndex])
	{
		if(neighbor == pointIndex)
			continue;

		float dist = glm::distance(m_particles.Positions[pointIndex].vec, m_particles.Positions[neighbor].vec); //TODO distance2
		float weight = kernal.Value(dist);
		sum += weight;
	}

	return sum;
}

void SPHSolverCPU::pressureForces()
{

	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	auto& p = m_particles.Pressures;
	auto& f = m_particles.Forces;

	float targetDensity = m_targetDensitiy;
	float eosScale = targetDensity * (SPEED_OF_SOUND * SPEED_OF_SOUND);
	constexpr float eosExponent = 7.0f;

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Pressures.size(); ++i)
	{
		p[i] = computePressure(d[i], targetDensity, eosScale, eosExponent, m_negativePressureScale);
		//p[index] = 50.0f * (d[index] - 82.0f);
	}


	accumlatePressureForces(x,d,p,f);
}

void SPHSolverCPU::accumlatePressureForces(const std::vector<ParticlePoint>& positions,const std::vector<float>& densities, std::vector<float>& pressures, std::vector<ParticlePoint>& forces)
{
	const float massSquared = m_mass * m_mass;
	SpikedKernel kernal = SpikedKernel(KERNEL_RADIUS);

	//now accumlate pressure
	#pragma omp parallel for
	for (int i = 0; i < pressures.size(); ++i)
	{
		for(const auto& neighbor : m_neighborList[i])
		{
			if(neighbor == i)
				continue;

			float dist = glm::distance(positions[i].vec, positions[neighbor].vec); //TODO distance2
			if(dist > 0.0f)
			{
				glm::vec3 direction = (positions[neighbor].vec - positions[i].vec) / dist;
				glm::vec3 force = massSquared
					* (pressures[i] / (densities[i] * densities[i])
						+ pressures[neighbor] / (densities[neighbor] * densities[neighbor]))
					* kernal.Gradiant(dist, direction);
				forces[i].vec -= force;
			}
		}
	}
}

float SPHSolverCPU::computePressure(float density,
	float targetDensity,
	float eosScale,
	float eosExponent,
	float negativePressureScale) const
{
	float p = eosScale / eosExponent
		* (pow((density / targetDensity), eosExponent) - 1.0f);

	//float p = 10.0f * (density - targetDensity);

	//negative pressue scaling
	if (p < 0)
		p *= negativePressureScale;
	return p;
}

void SPHSolverCPU::viscosityForces()
{
	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	const auto& v = m_particles.Velocities;
	auto& f = m_particles.Forces;

	float massSquared = m_mass * m_mass;
	SpikedKernel kernal = SpikedKernel(KERNEL_RADIUS);

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Forces.size(); ++i)
	{
		for(const auto& neighbor : m_neighborList[i])
		{
			if(neighbor == i)
				continue;

			float dist = glm::distance(m_particles.Positions[i].vec, m_particles.Positions[neighbor].vec); //TODO distance2

			f[i].vec += m_viscosityCoefficient * massSquared
				* (v[neighbor].vec - v[i].vec) / d[neighbor]
				* kernal.SecondDerivative(dist);
		}
	}
}


void SPHSolverCPU::resolveCollisions(std::vector<ParticlePoint>& positions, std::vector<ParticlePoint>& velocities)
{
	#pragma omp parallel
	{
	#pragma omp for
		for (int i = 0; i < PARTICLE_COUNT; i++)
		{
			glm::vec3& pos = positions[i].vec;
			glm::vec3& vel = velocities[i].vec;

			resolveCollision(m_particles.Positions[i].vec,pos,vel);
		}
	}
}

void SPHSolverCPU::resolveCollision(const glm::vec3& startPos, glm::vec3& pos, glm::vec3& vel)
{
	constexpr float RestitutionCoefficient = 0.2f;
	constexpr float frictionCoeffient = 0.1f;
	constexpr float radius = 0.1f;

	CollisionData collisionData{};

	//if(BOX_COLLIDER.GetAABB().IsSphereOutside(pos, radius))
	{
		//auto clostestPoint = BOX_COLLIDER.GetAABB().GetClosestPoint(pos,true);
		//add radius to velocity
		//auto velocityRadius = (vel * TIMESTEP) + clostestPoint.second * radius;
		//auto velocityRadius = ((vel - radius) * TIMESTEP);
		if (BOX_COLLIDER.CollisionOccured(pos, vel * TIMESTEP, collisionData))
		{
			glm::vec3 targetNormal = collisionData.CollisionNormal;
			auto targetPoint = collisionData.ContactPoint;

			float normalDotRelativeVelocity = glm::dot(targetNormal, vel);
			glm::vec3 relativeVelocityNormal = normalDotRelativeVelocity * targetNormal;
			glm::vec3 relativeVelocityT = vel - relativeVelocityNormal;

			// Check if the velocity is facing opposite direction of the surface
			// normal
			if (normalDotRelativeVelocity < 0.0f)
			{
				//Apply restitution coefficient to the surface normal component of the velocity
				glm::vec3 deltaRelativeVelocityNormal = (-RestitutionCoefficient - 1.0f) * relativeVelocityNormal;
				relativeVelocityNormal *= -RestitutionCoefficient;

				// Apply friction to the tangential component of the velocity
				if (glm::length2(relativeVelocityT) > 0.0f)
				{
					float frictionScale = std::max(1.0f - frictionCoeffient *
						glm::length(deltaRelativeVelocityNormal) /  glm::length(relativeVelocityT), 0.0f);
					relativeVelocityT *= frictionScale;
				}

				// Reassemble the components
				vel = relativeVelocityNormal + relativeVelocityT;
			}
			pos = targetPoint;
		}
	}

}

glm::vec3 lerp(glm::vec3 x, glm::vec3 y, float t) {
	return x * (1.f - t) + y * t;
}


void SPHSolverCPU::fakeViscosity()
{
	const auto& x = m_particles.Positions;
	const auto& d = m_particles.Densities;
	auto& v = m_particles.Velocities;

	SpikedKernel kernel = SpikedKernel(KERNEL_RADIUS);
	std::vector<glm::vec3> smoothedVelocities(m_particles.Size());

	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		float weightSum = 0.0f;
		glm::vec3 smoothedVelocity = glm::vec3(0.0f);

		for(const auto& neighbor : m_neighborList[i])
		{
			float distance = glm::distance(x[i].vec, x[neighbor].vec);
			float wj = m_mass / d[neighbor] * kernel.Value(distance);
			weightSum += wj;
			smoothedVelocity += wj * v[neighbor].vec;
		}

		float wi = m_mass / d[i];
		weightSum += wi;
		smoothedVelocity += wi * v[i].vec;

		if (weightSum > 0.0)
		{
			smoothedVelocity /= weightSum;
		}

		smoothedVelocities[i] = smoothedVelocity;
	}

	float factor = TIMESTEP * PSEUDO_VISCOSITY_COEFFICIENT;
	factor = std::max(0.0f, std::min(factor, 1.0f));
	#pragma omp parallel for
	for (int i = 0; i < m_particles.Size(); ++i)
	{
		v[1].vec = lerp(v[i].vec,smoothedVelocities[i], factor);
	}
}

void SPHSolverCPU::setupOneSidedParticles()
{
	const int perRow = static_cast<int>(floorf(cbrtf(PARTICLE_COUNT)));

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = (i % perRow);
		float y = (i / perRow) / perRow;
		float z = ((i / perRow) % perRow);

		x = Util::MapValue(x, 0, perRow, BOX_COLLIDER.GetAABB().Min().x, 0.0f);
		y = Util::MapValue(y, 0, perRow, BOX_COLLIDER.GetAABB().Min().y, BOX_COLLIDER.GetAABB().Max().y);
		z = Util::MapValue(z, 0, perRow, BOX_COLLIDER.GetAABB().Min().z, BOX_COLLIDER.GetAABB().Max().z);

		m_particles.Positions[i].vec = glm::vec3(x, y, z);
	}
}

void SPHSolverCPU::setupTwoSidedParticles()
{
	const int halfCount = PARTICLE_COUNT / 2;
	const int perRow = static_cast<int>(floorf(cbrtf(halfCount)));

	for (int i = 0; i < PARTICLE_COUNT; i++)
	{


		//Setup Right Side
		if (i < PARTICLE_COUNT / 2)
		{
			float x = (i % perRow);
			float y = (i / perRow) / perRow;
			float z = ((i / perRow) % perRow);
			
			x = Util::MapValue(x, 0, perRow, BOX_COLLIDER.GetAABB().Min().x, -BOX_COLLIDER.GetAABB().Width() * 0.33f);
			y = Util::MapValue(y, 0, perRow, BOX_COLLIDER.GetAABB().Min().y, BOX_COLLIDER.GetAABB().Max().y * 0.75f);
			z = Util::MapValue(z, 0, perRow, BOX_COLLIDER.GetAABB().Min().z, 0.0f);
			m_particles.Positions[i].vec = glm::vec3(x, y, z);

		}
		else
		{
			float x = ((i - halfCount) % perRow);
			float y = ((i - halfCount) / perRow) / perRow;
			float z = (((i - halfCount) / perRow) % perRow);
			
			x = Util::MapValue(x, 0, perRow, BOX_COLLIDER.GetAABB().Width() * 0.33f, BOX_COLLIDER.GetAABB().Max().x);
			y = Util::MapValue(y, 0, perRow, BOX_COLLIDER.GetAABB().Min().y, BOX_COLLIDER.GetAABB().Max().y * 0.75f);
			z = Util::MapValue(z, 0, perRow, 0.0f, BOX_COLLIDER.GetAABB().Max().z);
			m_particles.Positions[i].vec = glm::vec3(x, y, z);

		}
	}
}

void SPHSolverCPU::setupFillParticles()
{
	const int perRow = static_cast<int>(floorf(cbrtf(PARTICLE_COUNT)));
	
	for (int i = 0; i < PARTICLE_COUNT; i++)
	{
		float x = (i % perRow);
		float y = (i / perRow) / perRow;
		float z = ((i / perRow) % perRow);

		x = Util::MapValue(x, 0, perRow, BOX_COLLIDER.GetAABB().Min().x, BOX_COLLIDER.GetAABB().Max().x);
		y = Util::MapValue(y, 0, perRow, BOX_COLLIDER.GetAABB().Min().y, BOX_COLLIDER.GetAABB().Max().y);
		z = Util::MapValue(z, 0, perRow, BOX_COLLIDER.GetAABB().Min().z, BOX_COLLIDER.GetAABB().Max().z);
		
		m_particles.Positions[i].vec = glm::vec3(x, y, z);
	}
}

