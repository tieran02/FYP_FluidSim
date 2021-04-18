#pragma once
#include <renderer/Renderer.h>
#include <sph/SPHSolverCPU.h>
#include <sph/PCISPHSolverCPU.h>
#include <math/PlaneCollider.h>
#include <math/BoxCollider.h>
#include "opencl/OpenCLContext.h"
#include "opencl/OpenCLProgram.h"
#include "renderer/Framebuffer.h"
#include "sph/PCISPHSolverGPU.h"

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

	Camera m_camera{glm::vec3(0,5.0f,-20.5f),0.1f,50.0f,65.0f};
	QuadPrimitive plane{ glm::vec2(5.0f,5.0f)};
	BoxCollider boxCollider{glm::vec3{-2.5,-2.5,-2.5}, glm::vec3{2.5,2.5,2.5}};
	std::array<Transform,6> m_planeTransforms;

	SpherePrimitive sphere{0.1f,24,16};
	Shader shader, m_instancedShader, m_composeShader, m_depthShader, m_blurShader;

	const size_t SPHERE_COUNT{10240};
	//SPHSolverCPU m_solver{ 1.0/150.0, SPHERE_COUNT,boxCollider};
	//PCISPHSolverCPU m_solver{ 1.0/60.0, SPHERE_COUNT,boxCollider};
	PCISPHSolverGPU m_solver{ 1.0f/60.0f, SPHERE_COUNT,boxCollider, m_openCLContext};

	std::vector<Buffer> m_storageBuffers;
	Mesh m_fullscreenQuadMesh;
	FrameBuffer m_backgroundFrameBuffer;
	FrameBuffer m_depthFrameBuffer;

	void createRenderResources();
	void restart();
	void drawParticles(const Shader& shader);
	void drawBackground();
};
