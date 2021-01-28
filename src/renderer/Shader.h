#pragma once
#include <string>
#include <glad/glad.h>

class Shader
{
 public:
	Shader(std::string&& vertexPath,std::string&& fragmentPath);
	~Shader();

	GLuint ID() const {return m_shaderID;}
 private:
	GLuint CompileStage(const GLchar* code, GLenum stage);

	GLuint m_shaderID;
};
