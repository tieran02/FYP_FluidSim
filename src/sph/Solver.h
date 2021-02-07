#pragma once

#include <glm.hpp>

class Solver
{
 public:
	Solver(float timeStep);
	virtual ~Solver();
	void Update();

 protected:
	virtual void BeginTimeStep() = 0;
	virtual void ApplyForces() = 0;
	virtual void Integrate() = 0;
	virtual void ResolveCollisions() = 0;
	virtual void EndTimeStep() = 0;

	const float TIMESTEP;
 private:
	float m_tickTime;
};