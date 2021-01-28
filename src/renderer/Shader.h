#pragma once
#include <string>
#include <glad/glad.h>


class Shader
{
 public:
	Shader();
	~Shader();

	void Build(std::string&& vertexPath,std::string&& fragmentPath);
	void Bind();
	void Unbind();
	GLuint ID() const {return m_shaderID;}
 private:
	GLuint CompileStage(const GLchar* code, GLenum stage);

	GLuint m_shaderID;
};
