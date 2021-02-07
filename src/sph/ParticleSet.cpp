#include "ParticleSet.h"

void ParticleSet::Integrate(const ParticleState& state)
{
	Positions = std::move(state.Positions);
	Velocities = std::move(state.Velocities);
}
