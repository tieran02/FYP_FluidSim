#pragma once
#include <vector>
#include <glm.hpp>

struct ParticleSet
{
	ParticleSet(size_t particleCount)
	{
		Positions = std::vector<glm::vec3>{particleCount};
		Velocities = std::vector<glm::vec3>{particleCount};
		Forces = std::vector<glm::vec3>{particleCount};
	}

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
	std::vector<glm::vec3> Forces;
};


