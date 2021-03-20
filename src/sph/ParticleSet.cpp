#include "ParticleSet.h"

void ParticleSet::Integrate(ParticleState& state)
{
	Positions = std::move(state.Positions);
	Velocities = std::move(state.Velocities);
}

void ParticleSet::Reset()
{
	std::fill(Positions.begin(), Positions.end(), ParticlePoint());
	std::fill(Velocities.begin(), Velocities.end(), ParticlePoint());
	std::fill(Forces.begin(), Forces.end(), ParticlePoint());
}
