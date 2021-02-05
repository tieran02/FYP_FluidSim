#pragma once
#include <renderer/Renderer.h>
#include <sph/SPHSolverCPU.h>

class Simulation
{
 public:
	Simulation(Renderer& renderer);
	void Update();
 private:
	Renderer& m_renderer;

	const size_t SPHERE_COUNT{100000};
	SPHSolverCPU m_solver{1.0/60.0, SPHERE_COUNT};
	std::vector<glm::vec3> m_instancedPositions{ SPHERE_COUNT };
	Buffer particleBuffer{BufferType::STORAGE_BUFFER};

	Camera m_camera{glm::vec3(0,50.0f,-50.5f),1.0f,1000.0f,65.0f};
	PlanePrimitive plane{glm::vec2(10.0f,10.0f)};
	SpherePrimitive sphere{0.5f,24,16};
	Shader shader, m_instancedShader;

	void createRenderResources();
};
