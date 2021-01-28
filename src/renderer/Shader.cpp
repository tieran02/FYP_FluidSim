#include <iostream>
#include "Shader.h"
#include "Util.h"

Shader::Shader() : m_shaderID{0}
{

}

Shader::~Shader()
{
	glDeleteProgram(m_shaderID);
}

void Shader::Build(std::string&& vertexPath, std::string&& fragmentPath)
{
	std::string vertexCode = Util::ReadFile(vertexPath);
	std::string fragmentCode = Util::ReadFile(fragmentPath);

	GLuint vertexStage = CompileStage(vertexCode.c_str(), GL_VERTEX_SHADER);
	GLuint fragmentStage = CompileStage(fragmentCode.c_str(), GL_FRAGMENT_SHADER);

	m_shaderID = glCreateProgram();
	glAttachShader(m_shaderID, vertexStage);
	glAttachShader(m_shaderID, fragmentStage);
	glLinkProgram(m_shaderID);

	glDeleteShader(vertexStage);
	glDeleteShader(fragmentStage);
}

void Shader::Bind()
{
	glUseProgram(m_shaderID);
}
void Shader::Unbind()
{
	glUseProgram(0);
}

GLuint Shader::CompileStage(const GLchar* code, GLenum stage)
{
	GLuint shader = 0;
	int success;
	char infoLog[512];

	if(stage != GL_VERTEX_SHADER && stage != GL_FRAGMENT_SHADER)
	{
		std::cout << "Shader stage is unknown" << std::endl;
		return shader;
	}

	shader = glCreateShader(stage);
	glShaderSource(shader,1, &code, nullptr);
	glCompileShader(shader);

	// print compile errors if any
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

	if(!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	return shader;
}

