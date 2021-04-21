#pragma once
#include <cstdint>

#include "glad/gl.h"
#include <NonCopyable.h>
#include <NonMovable.h>

class FrameBuffer : NonCopyable
{
public:
	FrameBuffer(GLenum foramt, GLenum internalFormat);
	~FrameBuffer();
	FrameBuffer(FrameBuffer&& other) noexcept;

	void Create(uint32_t width, uint32_t height);
	void Bind() const;
	void Unbind() const;
	
	GLuint TextureID() const { return m_textureID; }
	GLuint FrameBufferID() const { return m_frameBufferID; }
	bool Valid() const { return m_textureID > 0; }
private:
	GLuint m_frameBufferID{ 0 };
	GLuint m_textureID{ 0 };
	GLuint m_renderBufferObject{ 0 };
	uint32_t m_width, m_height;
	GLenum m_format, m_internalFormat;

	void createTextureBuffer(uint32_t width, uint32_t height, GLenum format, GLenum internalFormat);
};
