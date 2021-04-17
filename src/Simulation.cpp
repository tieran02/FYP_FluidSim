#include <util/Log.h>
#include "Simulation.h"

Simulation::Simulation(Renderer& renderer) : m_renderer(renderer), m_isPaused(false)
{
	m_camera.LootAt(glm::vec3(0,25.0f,0.0f));
	createRenderResources();

	m_planeTransforms[0].SetRotation(glm::vec3(1,0,0),glm::radians(90.0f)); //floor
	m_planeTransforms[0].SetPosition(glm::vec3(0,-2.5f,0));

	m_planeTransforms[1].SetRotation(glm::vec3(1,0,0),glm::radians(-90.0f)); //ceil
	m_planeTransforms[1].SetPosition(glm::vec3(0,2.5f,0));

	m_planeTransforms[2].SetRotation(glm::vec3(1,0,0),glm::radians(0.0f)); //back
	m_planeTransforms[2].SetPosition(glm::vec3(0,0,2.5f));

	m_planeTransforms[3].SetRotation(glm::vec3(0,1,0),glm::radians(180.0f)); //front
	m_planeTransforms[3].SetPosition(glm::vec3(0,0,-2.5f));

	m_planeTransforms[4].SetRotation(glm::vec3(0,1,0),glm::radians(90.0f)); //left
	m_planeTransforms[4].SetPosition(glm::vec3(2.5f,0,0));

	m_planeTransforms[5].SetRotation(glm::vec3(0,1,0),glm::radians(-90.0f)); //right
	m_planeTransforms[5].SetPosition(glm::vec3(-2.5f,0,0));

	m_camera.LootAt(glm::vec3(0.0f));
}

void Simulation::Update()
{
	//run simulation
	if(!m_isPaused) 
		m_solver.Update();

	m_depthFrameBuffer.Bind();
	drawFrame();
	m_depthFrameBuffer.Unbind();


	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, m_depthFrameBuffer.TextureID());
	m_renderer.Draw(m_fullscreenQuadMesh, m_composeShader);
	glEnable(GL_DEPTH_TEST);

	m_renderer.EndFrame();
}

void Simulation::createRenderResources()
{
	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");
	m_instancedShader.Build("resources/shaders/teshShaderInstanced.vert","resources/shaders/testShader.frag");
	m_composeShader.Build("resources/shaders/compose.vert", "resources/shaders/compose.frag");
	
	plane.Build();
	sphere.Build();

	//	Buffer particleBuffer{BufferType::STORAGE_BUFFER};
	//   Buffer pressureBuffer{ BufferType::STORAGE_BUFFER };
	//build instanced buffer
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers.emplace_back(Buffer{ BufferType::STORAGE_BUFFER });
	m_storageBuffers[0].Build((void*)m_solver.Particles().Positions.data(),sizeof(ParticlePoint) * SPHERE_COUNT, 0);
	m_storageBuffers[1].Build((void*)m_solver.Particles().Pressures.data(), sizeof(float) * SPHERE_COUNT, 1);

	//set shader uniforms
	shader.Bind();
	shader.SetMat4("view", m_camera.ViewMatrix(), false);
	shader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	shader.SetVec4("ourColor", glm::vec4(1.0f,1.0f,1.0f,1.0f));
	shader.Unbind();

	//set shader uniforms
	m_instancedShader.Bind();
	m_instancedShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_instancedShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_instancedShader.SetVec4("ourColor", glm::vec4(0.0f,0.0f,1.0f,1.0f));
	m_instancedShader.Unbind();

	//Framebuffers
	m_depthFrameBuffer.Create(Window::Width(), Window::Height(), GL_RGB, GL_RGB32F);

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
}

void Simulation::restart()
{
	m_solver.Reset();
}

void Simulation::drawFrame()
{
	//Render
	m_renderer.BeginFrame();

	//update Camera views
	shader.Bind();
	shader.SetMat4("view", m_camera.ViewMatrix(), false);
	shader.Unbind();

	//set shader uniforms
	m_instancedShader.Bind();
	m_instancedShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_instancedShader.Unbind();


	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[0]);
	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[1]);
	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[2]);
	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[3]);
	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[4]);
	m_renderer.Draw(plane.GetMesh(), shader, m_planeTransforms[5]);

	//upload pos and pressure
	m_storageBuffers[0].Upload((void*)m_solver.Particles().Positions.data(), sizeof(ParticlePoint) * SPHERE_COUNT);
	m_storageBuffers[1].Upload((void*)m_solver.Particles().Pressures.data(), sizeof(float) * SPHERE_COUNT);

	m_renderer.DrawInstanced(sphere.GetMesh(), m_instancedShader, m_storageBuffers, SPHERE_COUNT);
}

void Simulation::KeyCallback(int key, int action, int mode)
{
	switch (key)
	{
	case GLFW_KEY_W:
		m_camera.Move(m_camera.Forward());
		break;
	case GLFW_KEY_S:
		m_camera.Move(-m_camera.Forward());
		break;
	case GLFW_KEY_A:
		m_camera.Move(-glm::cross(m_camera.Forward(), m_camera.Up()));
		break;
	case GLFW_KEY_D:
		m_camera.Move(glm::cross(m_camera.Forward(), m_camera.Up()));
		break;
	case GLFW_KEY_F5:
		restart();
		break;
	case GLFW_KEY_P:
		if(action == GLFW_RELEASE)
			m_isPaused = !m_isPaused;
		break;
	}
}

void Simulation::CursorCallback(double xPos, double yPos)
{
	m_camera.MousePosition(xPos,yPos);
}

void Simulation::MouseButtonCallback(int button, int action, int mod)
{

}



