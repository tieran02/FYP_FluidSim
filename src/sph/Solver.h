#pragma once

class Solver
{
 public:
	Solver(double timeStep);
	void Update();

 protected:
	virtual void BeginTimeStep() = 0;
	virtual void ApplyForces() = 0;
	virtual void Integrate() = 0;
	virtual void ResolveCollisions() = 0;
	virtual void EndTimeStep() = 0;
 private:
	double m_timestep;
	double m_tickTime;
};
