#include "Simulation.h"

Simulation::Simulation(Renderer& renderer) : m_renderer(renderer)
{
	createRenderResources();
}

void Simulation::Update()
{

	//Render
	m_renderer.BeginFrame();

	Transform planeTransform;
	planeTransform.SetRotation(glm::vec3(1,0,0),-1.5708f);
	m_renderer.Draw(plane.GetMesh(),shader,planeTransform);


	//update pos
	float timeValue = glfwGetTime();
	for (int i = 0; i < m_instancedSpheres.size(); ++i)
	{
		glm::mat4 trans = glm::mat4(1);
		trans = glm::translate(trans, glm::vec3(0,sin(timeValue + i), -2.5f + (i*1.1f)));
		m_instancedSpheres[i] = trans;
	}
	matrixBuffer.Upload(m_instancedSpheres.data(),sizeof(glm::mat4) * m_instancedSpheres.size());

	m_renderer.DrawInstanced(sphere.GetMesh(),m_instancedShader,matrixBuffer,m_instancedSpheres.data(),SPHERE_COUNT);

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
	Transform transform;
	for (int i = 0; i < m_instancedSpheres.size(); ++i)
	{
		transform.SetPosition(glm::vec3(0,0, -2.5f + (i*1.1f)));
		m_instancedSpheres[i] = transform.ModelMatrix();
	}

	matrixBuffer.Build(m_instancedSpheres.data(), sizeof(glm::mat4) * m_instancedSpheres.size());

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
