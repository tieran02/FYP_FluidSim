#pragma once
#include <cstdint>

#include "glad/gl.h"
#include <NonCopyable.h>
#include <NonMovable.h>
#include <memory>

class Texture;

class FrameBuffer : NonCopyable
{
public:
	FrameBuffer(GLenum foramt, GLenum internalFormat);
	~FrameBuffer();
	FrameBuffer(FrameBuffer&& other) noexcept;

	void Create(uint32_t width, uint32_t height);
	void Bind() const;
	void Unbind() const;

	const Texture* GetTexture() const { return m_texture.get(); }
	GLuint FrameBufferID() const { return m_frameBufferID; }
	bool Valid() const { return !m_texture; }
private:
	GLuint m_frameBufferID{ 0 };
	std::unique_ptr<Texture> m_texture;
	GLuint m_renderBufferObject{ 0 };
	uint32_t m_width, m_height;
	GLenum m_format, m_internalFormat;
};
