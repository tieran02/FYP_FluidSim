#pragma once
#include <string>
#include "glad/gl.h"
#include <glm.hpp>
#include <NonCopyable.h>

class Shader : NonCopyable
{
 public:
	Shader();
	~Shader();

	void Build(std::string&& vertexPath,std::string&& fragmentPath);
	void Bind();
	void Unbind();

	void SetBool(const std::string &name, bool value) const;
	void SetInt(const std::string &name, int value) const;
	void SetFloat(const std::string &name, float value) const;
	void SetVec2(const std::string &name, glm::vec2 value) const;
	void SetVec3(const std::string &name, glm::vec3 value) const;
	void SetVec4(const std::string &name, glm::vec4 value) const;
	void SetMat2(const std::string &name, const glm::mat2& value, bool transpose) const;
	void SetMat3(const std::string &name, const glm::mat3& value, bool transpose) const;
	void SetMat4(const std::string &name, const glm::mat4& value, bool transpose) const;


	GLuint ID() const {return m_shaderID;}
 private:
	GLuint CompileStage(const GLchar* code, GLenum stage);

	GLuint m_shaderID;
};
