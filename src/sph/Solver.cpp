#include "Solver.h"
#include <GLFW/glfw3.h>

Solver::Solver(double timeStep) : m_timestep(timeStep)
{
	m_tickTime = glfwGetTime();
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

		m_tickTime += m_timestep;
	}
}
