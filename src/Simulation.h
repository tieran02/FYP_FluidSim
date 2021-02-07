#pragma once
#include <renderer/Renderer.h>
#include <sph/SPHSolverCPU.h>
#include <math/PlaneCollider.h>

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

	Camera m_camera{glm::vec3(0,50.0f,-50.5f),0.1f,1000.0f,65.0f};
	QuadPrimitive plane{ glm::vec2(1000.0f,1000.0f)};
	PlaneCollider planeCollider{plane.GetPlane(),true};

	SpherePrimitive sphere{0.5f,24,16};
	Shader shader, m_instancedShader;

	const size_t SPHERE_COUNT{50000};
	SPHSolverCPU m_solver{1.0/30.0, SPHERE_COUNT,planeCollider};
	Buffer particleBuffer{BufferType::STORAGE_BUFFER};

	void createRenderResources();
};