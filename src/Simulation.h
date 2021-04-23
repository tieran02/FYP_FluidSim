#pragma once
#include <renderer/Renderer.h>
#include <sph/SPHSolverCPU.h>
#include <math/BoxCollider.h>
#include "opencl/OpenCLContext.h"
#include "renderer/FluidRenderer.h"
#include "sph/PCISPHSolverGPU.h"

class Simulation
{
 public:
	Simulation();
	void Init();
	void Update();

	void KeyCallback(int key, int action, int mode);
	void CursorCallback(double xPos, double yPos);
	void MouseButtonCallback(int key, int action, int mod);
 private:
	OpenCLContext m_openCLContext;

	bool m_isPaused;
	BoxCollider boxCollider{glm::vec3{-5.0f,-2.5f,-2.5f}, glm::vec3{5.0f,2.5f,2.5f}};

	const size_t SPHERE_COUNT{30720};
	//SPHSolverCPU m_solver{ 1.0/150.0, SPHERE_COUNT,boxCollider};
	//PCISPHSolverCPU m_solver{ 1.0/60.0, SPHERE_COUNT,boxCollider};
	std::unique_ptr<PCISPHSolverGPU> m_solver;
	std::unique_ptr<FluidRenderer> m_renderer;


	void restart();
};
