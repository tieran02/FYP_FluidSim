#include "Renderer.h"
#include <iostream>
#include <util/Log.h>

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight) : m_VAO(0)
{
	int initResult = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
	CORE_ASSERT(initResult, "Failed to init GLAD")
	LOG_CORE_INFO("Renderer using OpenGL 4.5");

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);

	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");

	plane.Build();
	sphere.Build();

	// We only need one VAO for now as we just use the Vertex struct as the layout,
	// if decide to have another vertex layout then another VAO will be needed.
	BuildVAO();
	Vertex::EnableAttributes();
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::DrawFrame() const
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.Bind();
	//set MVP
	shader.SetMat4("view", m_camera.ViewMatrix(), false);
	shader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);

	// update shader uniform
	float timeValue = glfwGetTime();

	glm::vec4 color{0.0f, 0.0f ,sin(timeValue) / 2.0f + 0.5f ,1.0f};

	Transform planeTransform;
	planeTransform.SetRotation(glm::vec3(1,0,0),-1.5708f);
	shader.SetVec4("ourColor", glm::vec4(1.0f,1.0f,1.0f,1.0f));
	plane.GetMesh().Draw(planeTransform, shader);

	shader.SetVec4("ourColor", color);
	sphere.GetMesh().Draw(Transform(), shader);

	shader.Unbind();
}

void Renderer::BuildVAO()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
}