#include <iostream>
#include "Shader.h"
#include "util/Util.h"

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

void Shader::Bind() const
{
	glUseProgram(m_shaderID);
}
void Shader::Unbind() const
{
	glUseProgram(0);
}

void Shader::SetBool(const std::string& name, bool value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform1i(location, static_cast<int>(value));
}

void Shader::SetInt(const std::string& name, int value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform1i(location, value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform1f(location, value);
}

void Shader::SetVec2(const std::string& name, glm::vec2 value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform2f(location, value.x, value.y);
}

void Shader::SetVec3(const std::string& name, glm::vec3 value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform3f(location, value.x, value.y, value.z);
}

void Shader::SetVec4(const std::string& name, glm::vec4 value) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::SetMat2(const std::string& name, const glm::mat2& value, bool transpose) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniformMatrix2fv(location,1, transpose, &value[0][0]);
}

void Shader::SetMat3(const std::string& name, const glm::mat3& value, bool transpose) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniformMatrix3fv(location,1, transpose, &value[0][0]);
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value, bool transpose) const
{
	int location = glGetUniformLocation(m_shaderID, name.c_str());
	glUniformMatrix4fv(location,1, transpose, &value[0][0]);
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
		std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
	};

	return shader;
}

