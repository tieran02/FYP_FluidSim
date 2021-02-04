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

Renderer::Renderer(uint32_t viewportWidth, uint32_t viewportHeight) : m_VAO(0), m_instancedVAO(0)
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
	BuildInstancedVAO();
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteVertexArrays(1, &m_instancedVAO);
}

void Renderer::BuildVAO()
{
	int attributeCount = 0;
	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);
	Vertex::EnableAttributes(attributeCount);
	glBindVertexArray(0);
}

void Renderer::BuildInstancedVAO()
{
	int attributeCount = 0;
	glGenVertexArrays(1, &m_instancedVAO);

	glBindVertexArray(m_instancedVAO);
	Vertex::EnableAttributes(attributeCount);

	// instance matrix
	glVertexBindingDivisor(1, 1);
	for(int i = 0; i < 4; i++)
	{
		//Model Matrices
		glEnableVertexAttribArray(attributeCount + i);
		glVertexAttribFormat(attributeCount + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * i * 4);
		glVertexAttribBinding(attributeCount + i, 1);
		//glVertexBindingDivisor(attributeCount + i, 1);
	}



	glBindVertexArray(0);
}

void Renderer::BeginFrame() const
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::EndFrame() const
{
	glBindVertexArray(0);
}

void Renderer::Draw(const Mesh& mesh, const Shader& shader, const Transform& transform) const
{
	glBindVertexArray(m_VAO);
	shader.Bind();
	mesh.Draw(transform,shader);
	shader.Unbind();
	glBindVertexArray(0);
}

void Renderer::DrawInstanced(const Mesh& mesh, const Shader& shader, const Buffer& instanceBuffer,
	const glm::mat4* instanceModelMats, size_t instanceCount) const
{
	glBindVertexArray(m_instancedVAO);
	shader.Bind();

	//update pos
	instanceBuffer.Upload((void*)instanceModelMats,sizeof(glm::mat4) * instanceCount);

	glBindVertexBuffer(0, mesh.VBO().ID(), 0, sizeof(Vertex));
	glBindVertexBuffer(1, instanceBuffer.ID(), 0, sizeof(glm::mat4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  mesh.EBO().ID());

	glDrawElementsInstanced(GL_TRIANGLES, mesh.Indices().size(), GL_UNSIGNED_INT, nullptr,instanceCount);

	shader.Unbind();
	glBindVertexArray(0);
}
