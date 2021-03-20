#pragma once
#include <vector>
#include <glm.hpp>

class ParticleState;


//Pos, Vel and Forces need to be a vec4 due to OpenCL float3 being 16 bytes
union ParticlePoint
{
	ParticlePoint() : vec4(glm::vec4(0.0f)) {}

	float data[4];
	glm::vec3 vec;
	glm::vec4 vec4;
};

struct ParticleSet
{
	ParticleSet(size_t particleCount)
	{
		Positions = std::vector<ParticlePoint>(particleCount);
		Velocities = std::vector<ParticlePoint>(particleCount);
		Forces = std::vector<ParticlePoint>(particleCount);
		Densities = std::vector<float>(particleCount);
		Pressures = std::vector<float>(particleCount);
	}

	void Integrate(ParticleState& state);
	void Reset();

	std::vector<ParticlePoint> Positions;
	std::vector<ParticlePoint> Velocities;
	std::vector<ParticlePoint> Forces;
	std::vector<float> Densities;
	std::vector<float> Pressures;

	size_t Size() const {return Positions.size();}
};

struct ParticleState
{
	ParticleState() = default;

	ParticleState(const ParticleSet& set)
	{
		Positions = set.Positions;
		Velocities = set.Velocities;
	}

	std::vector<ParticlePoint> Positions;
	std::vector<ParticlePoint> Velocities;
};

