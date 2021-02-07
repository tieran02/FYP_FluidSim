#pragma once
#include <vector>
#include <glm.hpp>

class ParticleState;

struct ParticleSet
{
	ParticleSet(size_t particleCount)
	{
		Positions = std::vector<glm::vec3>{particleCount};
		Velocities = std::vector<glm::vec3>{particleCount};
		Forces = std::vector<glm::vec3>{particleCount};
	}

	void Integrate(const ParticleState& state);

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
	std::vector<glm::vec3> Forces;
};

struct ParticleState
{
	ParticleState() = default;

	ParticleState(const ParticleSet& set)
	{
		Positions = set.Positions;
		Velocities = set.Velocities;
	}

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
};

