#pragma once
#include <renderer/Renderer.h>

class Simulation
{
 public:
	Simulation(Renderer& renderer);
	void Update();
 private:
	Renderer& m_renderer;

	void createRenderResources();

	const size_t SPHERE_COUNT{10000};
	std::vector<glm::mat4> m_instancedSpheres{ SPHERE_COUNT };
	Buffer matrixBuffer{BufferType::VERTEX_BUFFER};

	Camera m_camera{glm::vec3(0,0.5f,-2.5f),1.0f,100.0f,65.0f};
	PlanePrimitive plane{glm::vec2(10.0f,10.0f)};
	SpherePrimitive sphere{0.5f,24,16};
	Shader shader, m_instancedShader;
};
