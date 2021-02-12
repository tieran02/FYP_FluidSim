#include "Solver.h"
#include <GLFW/glfw3.h>
#include <chrono>

Solver::Solver(float timeStep) : TIMESTEP(timeStep)
{
	lag = std::chrono::nanoseconds(0);
	time_start = std::chrono::high_resolution_clock::now();
}

Solver::~Solver()
{

}

void Solver::Update()
{
	auto delta_time = std::chrono::high_resolution_clock::now() - time_start;
	time_start = std::chrono::high_resolution_clock::now();
	lag += std::chrono::duration_cast<std::chrono::nanoseconds>(delta_time);

	if (lag >= timestep)
	{
		lag -= timestep;

		BeginTimeStep();
		ApplyForces();
		Integrate();
		ResolveCollisions();
		EndTimeStep();
	}
}

