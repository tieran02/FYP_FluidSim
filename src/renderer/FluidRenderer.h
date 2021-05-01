#pragma once
#include "Framebuffer.h"
#include <array>
#include "Renderer.h"
#include "Texture.h"
#include "Camera.h"
#include "primitives/SpherePrimitive.h"

struct ParticleSet;

class FluidRenderer : public Renderer
{
public:
	FluidRenderer(uint32_t viewportWidth, uint32_t viewportHeight, uint32_t particleCount);
	~FluidRenderer() override;
	
	void Render();
	Camera& GetCamera();

	const Buffer& GetPositionStorageBuffer() const;
private:
	uint32_t m_particleCount;
	SpherePrimitive sphere{ 0.1f,24,16 };
	QuadPrimitive plane{ glm::vec2(5.0f,5.0f) };
	Camera m_camera{ glm::vec3(0,5.0f,-7.5f),0.1f,50.0f,65.0f };
	Shader m_depthShader, m_averagedNormalShader, m_normalShader, m_composeShader, m_sphereShader, m_bilateralBlurShader, m_gaussianBlurShader, m_defaultShader, m_skyboxShader, m_thicknessShader;
	FrameBuffer m_depthFBO{GL_RED, GL_RED};
	FrameBuffer m_averagedNormalFBO{GL_RGBA, GL_RGBA};
	FrameBuffer m_normalFBO{GL_RGBA, GL_RGBA};
	FrameBuffer m_thicknessFBO{ GL_RED, GL_RED };
	FrameBuffer m_backgroundFBO{GL_RGBA, GL_RGBA};
	std::vector<Buffer> m_storageBuffers;
	Mesh m_fullscreenQuadMesh;
	std::array<Transform, 6> m_planeTransforms;

	//OpenGL data
	Texture m_blurDepthTexture{GL_RED, GL_RED};
	Texture m_blurThicknessTexture{ GL_RED, GL_RED };
	Texture m_skyboxTexture{GL_RGB, GL_RGB};

	GLuint m_skyboxVAO{0}, m_skyboxVBO;

	void compileShaders();
	void createFrameBuffers();
	void updateShaderUniforms();
	void createTextures();
	void createSkyboxVAO();
	void drawSkybox();
	void uploadPositions();
	void drawParticles(const Shader& shader);
	void drawBox();

	
};
