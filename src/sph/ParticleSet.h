#pragma once
#include <vector>
#include <glm.hpp>

class ParticleState;

struct ParticleSet
{
	ParticleSet(size_t particleCount)
	{
		Positions = std::vector<glm::vec3>(particleCount);
		Velocities = std::vector<glm::vec3>(particleCount);
		Forces = std::vector<glm::vec3>(particleCount);
		Densities = std::vector<float>(particleCount);
		Pressures = std::vector<float>(particleCount);
	}

	void Integrate(ParticleState& state);
	void Reset();

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
	std::vector<glm::vec3> Forces;
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

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec3> Velocities;
};

