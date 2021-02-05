#include "Renderer.h"
#include <iostream>
#include <util/Log.h>

static void APIENTRY openglCallbackFunction(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
){
	(void)source; (void)type; (void)id;
	(void)severity; (void)length; (void)userParam;
	fprintf(stderr, "%s\n", message);
	if (severity==GL_DEBUG_SEVERITY_HIGH) {
		fprintf(stderr, "Aborting...\n");
		abort();
	}
}

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight) : m_VAO(0)
{
	int initResult = gladLoadGL((GLADloadfunc) glfwGetProcAddress);
	CORE_ASSERT(initResult, "Failed to init GLAD")
	LOG_CORE_INFO("Renderer using OpenGL 4.5");

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(openglCallbackFunction, nullptr);
	glDebugMessageControl(
		GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, true
	);

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// We only need one VAO for now as we just use the Vertex struct as the layout,
	// if decide to have another vertex layout then another VAO will be needed.
	BuildVAO();
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
}

void Renderer::BuildVAO()
{
	int attributeCount = 0;
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	Vertex::EnableAttributes(attributeCount);
	glBindVertexArray(0);
}

void Renderer::BeginFrame() const
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(m_VAO);
}

void Renderer::EndFrame() const
{
	glBindVertexArray(0);
}

void Renderer::Draw(const Mesh& mesh, const Shader& shader, const Transform& transform) const
{
	shader.Bind();
	mesh.Draw(transform,shader);
	shader.Unbind();
}

void Renderer::DrawInstanced(const Mesh& mesh, const Shader& shader, const Buffer& instanceBuffer, size_t instanceCount) const
{
	shader.Bind();

	mesh.VBO().Bind(0);
	instanceBuffer.Bind(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  mesh.EBO().ID());
	glDrawElementsInstanced(GL_TRIANGLES, mesh.Indices().size(), GL_UNSIGNED_INT, nullptr,instanceCount);

	shader.Unbind();
}
