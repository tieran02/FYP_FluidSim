#include "Renderer.h"
#include "Vertex.h"
#include <GLFW/glfw3.h>
#include <cassert>

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight) : m_VAO(0)
{
	assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);

	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");

	Vertex vertices[] =
	{
		{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)},
		{glm::vec3(0.0f,  0.75f, 0.0f), glm::vec3(0,0,0), glm::vec2(0,0)}
	};

	// We only need one VAO for now as we just use the Vertex struct as the layout,
	// if decide to have another vertex layout then another VAO will be needed.
	BuildVAO();
	vertexBuffer.Build(vertices, sizeof(vertices));

	Vertex::EnableAttributes();
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::DrawFrame()
{
	shader.Bind();

	// update shader uniform
	float timeValue = glfwGetTime();
	glm::vec4 color{0.0f, sin(timeValue) / 2.0f + 0.5f ,0.0f ,1.0f};
	shader.SetVec4("ourColor", color);

	// render the triangle
	glDrawArrays(GL_TRIANGLES, 0, 3);

	shader.Unbind();
}

void Renderer::BuildVAO()
{
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);


}