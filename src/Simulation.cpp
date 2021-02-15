#include <util/Log.h>
#include "Simulation.h"

Simulation::Simulation(Renderer& renderer) : m_renderer(renderer)
{
	m_camera.LootAt(glm::vec3(0,25.0f,0.0f));
	createRenderResources();

	m_planeTransforms[0].SetRotation(glm::vec3(1,0,0),glm::radians(90.0f)); //floor
	m_planeTransforms[0].SetPosition(glm::vec3(0,-5,0));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[0]), false));
	LOG_CORE_INFO("Floor Nornmal {}", glm::to_string(m_planeColliders[0].GetPlane().GetNormal()));

	m_planeTransforms[1].SetRotation(glm::vec3(1,0,0),glm::radians(-90.0f)); //ceil
	m_planeTransforms[1].SetPosition(glm::vec3(0,5,0));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[1]), false));
	LOG_CORE_INFO("Ceil Nornmal {}", glm::to_string(m_planeColliders[1].GetPlane().GetNormal()));

	m_planeTransforms[2].SetRotation(glm::vec3(1,0,0),glm::radians(0.0f)); //back
	m_planeTransforms[2].SetPosition(glm::vec3(0,0,5));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[2]), false));
	LOG_CORE_INFO("Back Nornmal {}", glm::to_string(m_planeColliders[2].GetPlane().GetNormal()));

	m_planeTransforms[3].SetRotation(glm::vec3(0,1,0),glm::radians(180.0f)); //front
	m_planeTransforms[3].SetPosition(glm::vec3(0,0,-5));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[3]), false));
	LOG_CORE_INFO("Front Nornmal {}", glm::to_string(m_planeColliders[3].GetPlane().GetNormal()));

	m_planeTransforms[4].SetRotation(glm::vec3(0,1,0),glm::radians(90.0f)); //left
	m_planeTransforms[4].SetPosition(glm::vec3(5,0,0));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[4]), false));
	LOG_CORE_INFO("Left Nornmal {}", glm::to_string(m_planeColliders[4].GetPlane().GetNormal()));

	m_planeTransforms[5].SetRotation(glm::vec3(0,1,0),glm::radians(-90.0f)); //right
	m_planeTransforms[5].SetPosition(glm::vec3(-5,0,0));
	m_planeColliders.emplace_back(PlaneCollider(plane.GetPlane().TransformedPlane(m_planeTransforms[5]), false));
	LOG_CORE_INFO("Right Nornmal {}", glm::to_string(m_planeColliders[5].GetPlane().GetNormal()));

	m_solver.Setup();
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


	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[0]);
	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[1]);
	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[2]);
	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[3]);
	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[4]);
	m_renderer.Draw(plane.GetMesh(),shader,m_planeTransforms[5]);

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

void Simulation::restart()
{
	m_solver.Reset();
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
	}
}

void Simulation::CursorCallback(double xPos, double yPos)
{
	m_camera.MousePosition(xPos,yPos);
}

void Simulation::MouseButtonCallback(int button, int action, int mod)
{

}



