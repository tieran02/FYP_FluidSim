#include <util/Log.h>
#include "Simulation.h"

Simulation::Simulation(Renderer& renderer) : m_renderer(renderer)
{
	m_camera.LootAt(glm::vec3(0,25.0f,0.0f));
	createRenderResources();
}

void Simulation::Update()
{
	//run simulation
	m_solver.Update();

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

	Transform planeTransform;
	planeTransform.SetRotation(glm::vec3(1,0,0),-1.5708f);
	planeCollider.SetTransform(planeTransform);
	m_renderer.Draw(plane.GetMesh(),shader,planeTransform);

	//update pos
	particleBuffer.Upload((void*)m_solver.Particles().Positions.data(),sizeof(glm::vec3) * SPHERE_COUNT);

	m_renderer.DrawInstanced(sphere.GetMesh(),m_instancedShader,particleBuffer ,SPHERE_COUNT);

	m_renderer.EndFrame();
}

void Simulation::createRenderResources()
{
	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");
	m_instancedShader.Build("resources/shaders/teshShaderInstanced.vert","resources/shaders/testShader.frag");

	plane.Build();
	sphere.Build();

	//build instanced buffer
	particleBuffer.Build((void*)m_solver.Particles().Positions.data(),sizeof(glm::vec3) * SPHERE_COUNT);

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
	}
}

void Simulation::CursorCallback(double xPos, double yPos)
{
	m_camera.MousePosition(xPos,yPos);
}

void Simulation::MouseButtonCallback(int button, int action, int mod)
{

}


