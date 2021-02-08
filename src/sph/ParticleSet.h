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

	void Integrate(ParticleState& state);
	void Reset();

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
	std::vector<glm::vec3> Forces;

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

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
};

