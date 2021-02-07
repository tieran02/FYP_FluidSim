#include "Solver.h"
#include <GLFW/glfw3.h>

Solver::Solver(float timeStep) : TIMESTEP(timeStep)
{
	m_tickTime = glfwGetTime();
}

Solver::~Solver()
{

}

void Solver::Update()
{
	while (m_tickTime < glfwGetTime())
	{
		BeginTimeStep();
		ApplyForces();
		Integrate();
		ResolveCollisions();
		EndTimeStep();

		m_tickTime += TIMESTEP;
	}
}

