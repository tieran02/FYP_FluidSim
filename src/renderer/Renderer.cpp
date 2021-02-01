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

	std::vector<Vertex> vertices =
	{
		{glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3( -0.5f,  0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)}
	};

	std::vector<uint32_t> indices =
	{
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	mesh.Build(std::move(vertices),std::move(indices));

	Vertex vertices1[] =
	{
		{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(0.0f,  0.25f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)}
	};

	// We only need one VAO for now as we just use the Vertex struct as the layout,
	// if decide to have another vertex layout then another VAO will be needed.
	BuildVAO();
	Vertex::EnableAttributes();

	vertexBuffer1.Build(vertices1, 3 * sizeof(Vertex));
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::DrawFrame()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.Bind();

	// update shader uniform
	float timeValue = glfwGetTime();

	if(static_cast<int>(std::floor(timeValue)) % 2 == 0)
	{
		glm::vec4 color{0.0f, sin(timeValue) / 2.0f + 0.5f ,0.0f ,1.0f};
		shader.SetVec4("ourColor", color);
		mesh.Draw();
	}
	else
	{
		glm::vec4 color{0.0f, 0.0f ,sin(timeValue) / 2.0f + 0.5f ,1.0f};
		shader.SetVec4("ourColor", color);
		vertexBuffer1.Bind();
		//render the triangle
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	shader.Unbind();
}

void Renderer::BuildVAO()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
}