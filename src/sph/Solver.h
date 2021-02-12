#pragma once

#include <glm.hpp>
#include <chrono>

using namespace std::chrono_literals;

class Solver
{
 public:
	Solver(float timeStep);
	virtual ~Solver();
	void Update();
	virtual void Setup() = 0;
	virtual void Reset() = 0;

 protected:
	virtual void BeginTimeStep() = 0;
	virtual void ApplyForces() = 0;
	virtual void Integrate() = 0;
	virtual void ResolveCollisions() = 0;
	virtual void EndTimeStep() = 0;

	const float TIMESTEP;
 private:
	const std::chrono::nanoseconds timestep{33ms};

	std::chrono::time_point<std::chrono::high_resolution_clock> time_start;
	std::chrono::nanoseconds lag;
};
