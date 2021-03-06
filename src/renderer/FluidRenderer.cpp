#include "FluidRenderer.h"

#include "sph/ParticleSet.h"
#include "Texture.h"

constexpr float skyboxVertices[] = {
	// positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

FluidRenderer::FluidRenderer(uint32_t viewportWidth, uint32_t viewportHeight, uint32_t particleCount) : Renderer(viewportWidth, viewportHeight),
	m_particleCount(particleCount),
	m_renderMode(RenderMode::FLUID)
{
	m_camera.LootAt(glm::vec3(0, 25.0f, 0.0f));
	compileShaders();
	createFrameBuffers();
	createTextures();
	createSkyboxVAO();

	sphere.Build();
	plane.Build();

	//build instanced buffer
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers[0].Build(nullptr, sizeof(ParticlePoint) * particleCount, 0);
	m_storageBuffers[1].Build(nullptr, sizeof(float) * particleCount, 1);

	//Create fullscreen quad to combine the FBOs
	std::vector<Vertex> quadVerts =
	{
		Vertex(glm::vec3(-1.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec2(0.0,1.0f)),
		Vertex(glm::vec3(-1.0f,-1.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec2(0.0,0.0f)),
		Vertex(glm::vec3(1.0f,-1.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec2(1.0,0.0f)),
		Vertex(glm::vec3(1.0f,1.0f,0.0f), glm::vec3(0.0f,0.0f,-1.0f), glm::vec2(1.0,1.0f))
	};

	std::vector<uint32_t> quadIndices =
	{
		0,1,2,
		0,2,3
	};

	m_fullscreenQuadMesh.Build(std::move(quadVerts), std::move(quadIndices));

	m_planeTransforms[0].SetRotation(glm::vec3(1, 0, 0), glm::radians(90.0f)); //floor
	m_planeTransforms[0].SetPosition(glm::vec3(0, -2.5f, 0));
	m_planeTransforms[1].SetRotation(glm::vec3(1, 0, 0), glm::radians(-90.0f)); //ceil
	m_planeTransforms[1].SetPosition(glm::vec3(0, 2.5f, 0));
	m_planeTransforms[2].SetRotation(glm::vec3(1, 0, 0), glm::radians(0.0f)); //back
	m_planeTransforms[2].SetPosition(glm::vec3(0, 0, 2.5f));
	m_planeTransforms[3].SetRotation(glm::vec3(0, 1, 0), glm::radians(180.0f)); //front
	m_planeTransforms[3].SetPosition(glm::vec3(0, 0, -2.5f));
	m_planeTransforms[4].SetRotation(glm::vec3(0, 1, 0), glm::radians(90.0f)); //left
	m_planeTransforms[4].SetPosition(glm::vec3(2.5f, 0, 0));
	m_planeTransforms[5].SetRotation(glm::vec3(0, 1, 0), glm::radians(-90.0f)); //right
	m_planeTransforms[5].SetPosition(glm::vec3(-2.5f, 0, 0));
	
	m_camera.LootAt(glm::vec3(0, 0.0f, 0.0f));
}

FluidRenderer::~FluidRenderer()
{
	if(m_skyboxVBO != 0)
		glDeleteBuffers(1, &m_skyboxVBO);

	if(m_skyboxVAO != 0)
		glDeleteVertexArrays(1, &m_skyboxVAO);
}

void FluidRenderer::Render()
{
	uploadPositions();
	updateShaderUniforms();

	BeginFrame();

	switch (m_renderMode)
	{
	case RenderMode::SPHERE:
		sphereRender();
		break;
	case RenderMode::FLUID:
		fluidRender();
		break;
	case RenderMode::NORMAL:
		normalRender();
		break;
	case RenderMode::DEPTH:
		depthRender();
		break;
	case RenderMode::THICKNESS:
		thicknessRender();
		break;
	default: ;
	}

	

	EndFrame();
}

Camera& FluidRenderer::GetCamera()
{
	return m_camera;
}

const Buffer& FluidRenderer::GetPositionStorageBuffer() const
{
	return m_storageBuffers[0];
}

const Buffer& FluidRenderer::GetPressureStorageBuffer() const
{
	return m_storageBuffers[1];
}

void FluidRenderer::ChangeRenderMode(RenderMode mode)
{
	m_renderMode = mode;
}

void FluidRenderer::compileShaders()
{
	m_defaultShader.Build("resources/shaders/testShader.vert", "resources/shaders/testShader.frag");
	m_sphereShader.Build("resources/shaders/teshShaderInstanced.vert", "resources/shaders/testShader.frag");
	m_depthShader.Build("resources/shaders/teshShaderInstanced.vert", "resources/shaders/depth.frag");
	m_composeShader.Build("resources/shaders/compose.vert", "resources/shaders/compose.frag");
	m_averagedNormalShader.Build("resources/shaders/compose.vert", "resources/shaders/screenSpaceAvgNormal.frag");
	m_bilateralBlurShader.Build("resources/shaders/compose.vert", "resources/shaders/bilateralBlur.frag");
	m_gaussianBlurShader.Build("resources/shaders/compose.vert", "resources/shaders/blur.frag");
	m_skyboxShader.Build("resources/shaders/skybox.vert", "resources/shaders/skybox.frag");
	m_thicknessShader.Build("resources/shaders/teshShaderInstanced.vert", "resources/shaders/thickness.frag");


	//set shader uniforms
	m_defaultShader.Bind();
	m_defaultShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_defaultShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_defaultShader.Unbind();
	
	m_sphereShader.Bind();
	m_sphereShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_sphereShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_sphereShader.Unbind();

	m_depthShader.Bind();
	m_depthShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_depthShader.Unbind();

	m_thicknessShader.Bind();
	m_thicknessShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_thicknessShader.Unbind();

	m_averagedNormalShader.Bind();
	m_averagedNormalShader.SetMat4("projection", m_camera.PerspectiveMatrix(), false);
	m_averagedNormalShader.SetVec2("screenSize", glm::vec2(Window::Width(), Window::Height()));
	m_averagedNormalShader.SetFloat("ProjectFov", m_camera.FOV());
	m_averagedNormalShader.Unbind();

	m_composeShader.Bind();
	m_composeShader.SetMat4("projection", m_camera.PerspectiveMatrix(), false);
	m_composeShader.SetVec2("screenSize", glm::vec2(Window::Width(), Window::Height()));
	m_composeShader.SetVec4("waterColor", glm::vec4(0.1f, 0.4f, 0.8f, 1.0f));
	m_composeShader.Unbind();

	m_skyboxShader.Bind();
	m_skyboxShader.SetMat4("projection", m_camera.PerspectiveMatrix(), false);
	m_defaultShader.Unbind();

}

void FluidRenderer::createFrameBuffers()
{
	//Framebuffers
	m_depthFBO.Create(Window::Width(), Window::Height());
	m_averagedNormalFBO.Create(Window::Width(), Window::Height());
	m_normalFBO.Create(Window::Width(), Window::Height());
	m_thicknessFBO.Create(Window::Width(), Window::Height());
	m_backgroundFBO.Create(Window::Width(), Window::Height());
}

void FluidRenderer::updateShaderUniforms()
{
	//update Camera views
	glm::mat3 invView = glm::mat3(glm::inverse(m_camera.ViewMatrix()));
	
	m_defaultShader.Bind();
	m_defaultShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_defaultShader.Unbind();
	m_sphereShader.Bind();
	m_sphereShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_sphereShader.Unbind();
	m_depthShader.Bind();
	m_depthShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_depthShader.Unbind();
	m_averagedNormalShader.Bind();
	m_averagedNormalShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_averagedNormalShader.Unbind();
	m_thicknessShader.Bind();
	m_thicknessShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_thicknessShader.Unbind();
	m_skyboxShader.Bind();
	m_skyboxShader.SetMat4("view", glm::mat4(glm::mat3(m_camera.ViewMatrix())), false);
	m_skyboxShader.Unbind();
	
	m_composeShader.Bind();
	m_composeShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_composeShader.SetVec2("clipPositionToEye", glm::vec2(tanf(m_camera.FOV() * 0.5f) * (Window::Width() / Window::Height()), tanf(m_camera.FOV() * 0.5f)));
	m_composeShader.SetMat3("invView3", invView, false);
	m_composeShader.Unbind();
}

void FluidRenderer::createTextures()
{
	if (m_blurDepthTexture.TextureID() != 0)
		return;

	m_blurDepthTexture.CreateEmptyTexture2D(Window::Width(), Window::Height());
	m_blurThicknessTexture.CreateEmptyTexture2D(Window::Width(), Window::Height());

	m_skyboxTexture.CreateCubemapFromFile
	(
	{
		"resources/textures/skybox/posx.jpg",
		"resources/textures/skybox/negx.jpg",
		"resources/textures/skybox/posy.jpg",
		"resources/textures/skybox/negy.jpg",
		"resources/textures/skybox/posz.jpg",
		"resources/textures/skybox/negz.jpg",
		 }
	);
}

void FluidRenderer::createSkyboxVAO()
{
	glGenVertexArrays(1, &m_skyboxVAO);
	glGenBuffers(1, &m_skyboxVBO);
	glBindVertexArray(m_skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	glBindVertexArray(0);
}


void FluidRenderer::uploadPositions()
{
	//upload pos and pressure
	//m_storageBuffers[0].Upload((void*)m_particles.Positions.data(), sizeof(ParticlePoint) * m_particles.Size());
	//m_storageBuffers[1].Upload((void*)m_particles.Pressures.data(), sizeof(float) * m_particles.Size());
}

void FluidRenderer::drawParticles(const Shader& shader)
{
	DrawInstanced(sphere.GetMesh(), shader, m_storageBuffers, m_particleCount);
}

void FluidRenderer::drawBox()
{
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[0]);
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[1]);
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[2]);
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[3]);
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[4]);
	Draw(plane.GetMesh(), m_defaultShader, m_planeTransforms[5]);
}

void FluidRenderer::fluidRender()
{
	//Draw skybox
	m_backgroundFBO.Bind();
	Clear();
	drawSkybox();
	m_backgroundFBO.Unbind();

	//Draw particles to depth buffer using the depth FBO
	m_depthFBO.Bind();
	Clear();
	drawParticles(m_depthShader);
	m_depthFBO.Unbind();

	//Blur depth texture into blurDepthTexture
	m_blurDepthTexture.CopyTexture(*m_depthFBO.GetTexture());
	m_blurDepthTexture.BlurTexture(m_bilateralBlurShader, m_fullscreenQuadMesh, 1);

	//Draw normals from the depth buffer into the normalFBO (Screen space normals)
	m_averagedNormalFBO.Bind();
	Clear();
	m_blurDepthTexture.Bind(0);
	Draw(m_fullscreenQuadMesh, m_averagedNormalShader);
	m_averagedNormalFBO.Unbind();

	//Render thickness without the depth test for addictive blend
	m_thicknessFBO.Bind();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);
	Clear();
	drawParticles(m_thicknessShader);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	m_thicknessFBO.Unbind();

	//Blur thickness texture into blurThicknessTexture
	m_blurThicknessTexture.CopyTexture(*m_thicknessFBO.GetTexture());
	m_blurThicknessTexture.BlurTexture(m_gaussianBlurShader, m_fullscreenQuadMesh, 8);

	//Draw final quad combining the FBOs
	m_blurDepthTexture.Bind(0);
	m_averagedNormalFBO.GetTexture()->Bind(1);
	m_backgroundFBO.GetTexture()->Bind(2);
	m_skyboxTexture.Bind(3);
	m_blurThicknessTexture.Bind(4);
	Draw(m_fullscreenQuadMesh, m_composeShader);
}

void FluidRenderer::sphereRender()
{
	//Draw particles to depth buffer using the depth FBO
	Clear();
	drawParticles(m_sphereShader);
}

void FluidRenderer::normalRender()
{
	//Draw particles to depth buffer using the depth FBO
	m_depthFBO.Bind();
	Clear();
	drawParticles(m_depthShader);
	m_depthFBO.Unbind();

	//Blur depth texture into blurDepthTexture
	m_blurDepthTexture.CopyTexture(*m_depthFBO.GetTexture());
	m_blurDepthTexture.BlurTexture(m_bilateralBlurShader, m_fullscreenQuadMesh, 1);

	//Draw normals from the depth buffer into the normalFBO (Screen space normals)
	Clear();
	m_blurDepthTexture.Bind(0);
	Draw(m_fullscreenQuadMesh, m_averagedNormalShader);
}


void FluidRenderer::depthRender()
{
	//Draw particles to depth buffer using the depth FBO
	Clear();
	drawParticles(m_depthShader);
}

void FluidRenderer::thicknessRender()
{
	//Render thickness without the depth test for addictive blend
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE);
	Clear();
	drawParticles(m_thicknessShader);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
}

void FluidRenderer::drawSkybox()
{
	glBindVertexArray(m_skyboxVAO);

	glDepthMask(GL_FALSE);
	m_skyboxShader.Bind();
	m_skyboxTexture.Bind(0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);

	glBindVertexArray(m_VAO);
}

