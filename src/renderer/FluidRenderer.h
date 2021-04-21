#pragma once
#include "Framebuffer.h"
#include "Renderer.h"
#include "Texture.h"

struct ParticleSet;

class FluidRenderer : public Renderer
{
public:
	FluidRenderer(uint32_t viewportWidth, uint32_t viewportHeight, const ParticleSet& particles);
	~FluidRenderer() override;
	
	void Render();
	Camera& GetCamera();
private:
	const ParticleSet& m_particles;
	SpherePrimitive sphere{ 0.1f,24,16 };
	QuadPrimitive plane{ glm::vec2(5.0f,5.0f) };
	Camera m_camera{ glm::vec3(0,5.0f,-7.5f),0.1f,50.0f,65.0f };
	Shader m_depthShader, m_averagedNormalShader, m_normalShader, m_composeShader, m_sphereShader, m_blurShader, m_defaultShader;
	FrameBuffer m_depthFBO{GL_RED, GL_RED};
	FrameBuffer m_averagedNormalFBO{GL_RGBA, GL_RGBA};
	FrameBuffer m_normalFBO{GL_RGBA, GL_RGBA};
	std::vector<Buffer> m_storageBuffers;
	Mesh m_fullscreenQuadMesh;
	std::array<Transform, 6> m_planeTransforms;

	//OpenGL data
	Texture m_blurDepthTexture{GL_RED, GL_RED};

	void compileShaders();
	void createFrameBuffers();
	void updateShaderUniforms();
	void createBlurDepthTexture();
	void uploadPositions();
	void drawParticles(const Shader& shader);
	void drawBox();
	
};
