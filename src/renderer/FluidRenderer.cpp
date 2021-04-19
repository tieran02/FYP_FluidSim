#include "FluidRenderer.h"
#include "sph/ParticleSet.h"

FluidRenderer::FluidRenderer(uint32_t viewportWidth, uint32_t viewportHeight, const ParticleSet& particles) : Renderer(viewportWidth, viewportHeight), m_particles(particles)
{
	m_camera.LootAt(glm::vec3(0, 25.0f, 0.0f));
	compileShaders();
	createFrameBuffers();

	sphere.Build();
	plane.Build();

	//build instanced buffer
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers[0].Build((void*)m_particles.Positions.data(), sizeof(ParticlePoint) * particles.Size(), 0);
	m_storageBuffers[1].Build((void*)m_particles.Pressures.data(), sizeof(float) * particles.Size(), 1);

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

void FluidRenderer::Render()
{
	uploadPositions();
	updateShaderUniforms();

	BeginFrame();
	
	//Draw particles to depth buffer using the depth FBO
	m_depthFBO.Bind();
	Clear();
	drawParticles(m_depthShader);
	m_depthFBO.Unbind();

	//Draw final quad combining the FBOs
	glDisable(GL_DEPTH_TEST);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_depthFBO.TextureID());
	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, m_backgroundFrameBuffer.TextureID());
	Draw(m_fullscreenQuadMesh, m_composeShader);
	glEnable(GL_DEPTH_TEST);

	EndFrame();
	
}

void FluidRenderer::compileShaders()
{
	m_defaultShader.Build("resources/shaders/testShader.vert", "resources/shaders/testShader.frag");
	m_sphereShader.Build("resources/shaders/teshShaderInstanced.vert", "resources/shaders/testShader.frag");
	m_depthShader.Build("resources/shaders/teshShaderInstanced.vert", "resources/shaders/depth.frag");
	m_composeShader.Build("resources/shaders/compose.vert", "resources/shaders/compose.frag");
	m_blurShader.Build("resources/shaders/compose.vert", "resources/shaders/blur.frag");

	//set shader uniforms
	m_defaultShader.Bind();
	m_defaultShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_defaultShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_defaultShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_defaultShader.Unbind();
	
	m_sphereShader.Bind();
	m_sphereShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_sphereShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_sphereShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_sphereShader.Unbind();

	m_depthShader.Bind();
	m_depthShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_depthShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_depthShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_depthShader.Unbind();

	m_composeShader.Bind();
	m_composeShader.SetMat4("projection", m_camera.PerspectiveMatrix(), false);
	m_composeShader.SetVec2("screenSize", glm::vec2(Window::Width(), Window::Height()));
	m_composeShader.SetVec4("ourColor", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	m_composeShader.Unbind();
}

void FluidRenderer::createFrameBuffers()
{
	//Framebuffers
	m_depthFBO.Create(Window::Width(), Window::Height(), GL_RED, GL_RED);
}

void FluidRenderer::updateShaderUniforms()
{
	//update Camera views
	m_defaultShader.Bind();
	m_defaultShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_defaultShader.Unbind();
	m_sphereShader.Bind();
	m_sphereShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_sphereShader.Unbind();
	m_depthShader.Bind();
	m_depthShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_depthShader.Unbind();
	m_composeShader.Bind();
	m_composeShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_composeShader.Unbind();
}

void FluidRenderer::uploadPositions()
{
	//upload pos and pressure
	m_storageBuffers[0].Upload((void*)m_particles.Positions.data(), sizeof(ParticlePoint) * m_particles.Size());
	m_storageBuffers[1].Upload((void*)m_particles.Pressures.data(), sizeof(float) * m_particles.Size());
}

void FluidRenderer::drawParticles(const Shader& shader)
{
	DrawInstanced(sphere.GetMesh(), shader, m_storageBuffers, m_particles.Size());
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
