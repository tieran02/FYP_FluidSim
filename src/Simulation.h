#pragma once
#include <renderer/Renderer.h>
#include <sph/SPHSolverCPU.h>
#include <sph/PCISPHSolverCPU.h>
#include <math/PlaneCollider.h>
#include <math/BoxCollider.h>
#include "opencl/OpenCLContext.h"

class Simulation
{
 public:
	Simulation(Renderer& renderer);
	void Update();

	void KeyCallback(int key, int action, int mode);
	void CursorCallback(double xPos, double yPos);
	void MouseButtonCallback(int key, int action, int mod);
 private:
	Renderer& m_renderer;
	OpenCLContext m_openCLContext;
	bool m_isPaused;

	Camera m_camera{glm::vec3(0,5.0f,-20.5f),0.1f,1000.0f,65.0f};
	QuadPrimitive plane{ glm::vec2(5.0f,5.0f)};
	BoxCollider boxCollider{glm::vec3{-2.5,-2.5,-2.5}, glm::vec3{2.5,2.5,2.5}};
	std::array<Transform,6> m_planeTransforms;

	SpherePrimitive sphere{0.1f,24,16};
	Shader shader, m_instancedShader;

	const size_t SPHERE_COUNT{20000};
	//SPHSolverCPU m_solver{ 1.0/150.0, SPHERE_COUNT,boxCollider};
	PCISPHSolverCPU m_solver{ 1.0/60.0, SPHERE_COUNT,boxCollider};
	Buffer particleBuffer{BufferType::STORAGE_BUFFER};

	void createRenderResources();
	void restart();
};
