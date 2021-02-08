#include "ParticleSet.h"

void ParticleSet::Integrate(ParticleState& state)
{
	Positions = std::move(state.Positions);
	Velocities = std::move(state.Velocities);
}

void ParticleSet::Reset()
{
	std::fill(Positions.begin(), Positions.end(), glm::vec3(0,0,0));
	std::fill(Velocities.begin(), Velocities.end(), glm::vec3(0, 0, 0));
	std::fill(Forces.begin(), Forces.end(), glm::vec3(0, 0, 0));
}
