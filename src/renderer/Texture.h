#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <NonCopyable.h>
#include <string>
#include "glad/gl.h"

class Shader;
class Mesh;

class Texture : NonCopyable
{
 public:
	Texture(GLenum format, GLenum internalFormat);
	~Texture();
	Texture(Texture&& other) noexcept;

	void CreateEmptyTexture2D(GLuint width, GLuint height);
	void CreateTextureFromFile(const std::string& path);
	void CopyTexture(const Texture& sourceTexture);

	void BlurTexture(const Shader& blurShader, const Mesh& quadMesh, uint32_t amount);
 private:
	GLuint m_textureID{0};
	GLenum m_format, m_internalFormat;
	GLuint m_width, m_height;


};
