#include "Renderer.h"
#include <GLFW/glfw3.h>
#include <cassert>

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight)
{
	assert(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);

	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		0.0f,  0.5f, 0.0f   // top
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(VAO);
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
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