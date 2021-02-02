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
		GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, true
	);

	// Define the viewport dimensions
	glViewport(0, 0, viewportWidth, viewportHeight);

	//build shader
	shader.Build("resources/shaders/testShader.vert","resources/shaders/testShader.frag");
	m_instancedShader.Build("resources/shaders/teshShaderInstanced.vert","resources/shaders/testShader.frag");

	plane.Build();
	sphere.Build();

	// We only need one VAO for now as we just use the Vertex struct as the layout,
	// if decide to have another vertex layout then another VAO will be needed.
	BuildVAO();
	BuildInstancedVAO();

	//build instanced buffer
	Transform transform;
	for (int i = 0; i < m_instancedSpheres.size(); ++i)
	{
		transform.SetPosition(glm::vec3(0,0, -2.5f + (i*1.1f)));
		m_instancedSpheres[i] = transform.ModelMatrix();
	}

	matrixBuffer.Build(m_instancedSpheres.data(), sizeof(glm::mat4) * m_instancedSpheres.size());
}

Renderer::~Renderer()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteVertexArrays(1, &m_instancedVAO);
}

void Renderer::DrawFrame() const
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindVertexArray(m_VAO);
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

	shader.Unbind();
	glBindVertexArray(0);

	//instanced rendering

	glBindVertexArray(m_instancedVAO);
	m_instancedShader.Bind();

	m_instancedShader.SetMat4("view", m_camera.ViewMatrix(), false);
	m_instancedShader.SetMat4("perspective", m_camera.PerspectiveMatrix(), false);
	m_instancedShader.SetVec4("ourColor", color);

	glBindVertexBuffer(0, sphere.GetMesh().VBO().ID(), 0, sizeof(Vertex));
	glBindVertexBuffer(1, matrixBuffer.ID(), 0, sizeof(glm::mat4));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,  sphere.GetMesh().EBO().ID());

	glDrawElementsInstanced(GL_TRIANGLES, sphere.GetMesh().Indices().size(), GL_UNSIGNED_INT, nullptr, m_instancedSpheres.size());

	m_instancedShader.Unbind();
	glBindVertexArray(0);

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
