#pragma once
#include "glad/gl.h"
#include <NonCopyable.h>
#include <NonMovable.h>

enum class BufferType
{
	VERTEX_BUFFER,
	ELEMENT_BUFFER,
	UNIFORM_BUFFER,
	STORAGE_BUFFER
};

class Buffer : NonCopyable
{
 public:
	Buffer(BufferType bufferType);
	~Buffer();
	Buffer(Buffer&& other) noexcept;

	void Build(void* data, size_t size, GLuint bindPoint = 0);
	void Bind(GLuint bindPoint = 0) const;
	void Unbind() const;

	void Upload(void* data, size_t size) const;

	GLuint ID() const {return m_id;}
	bool Valid() const {return m_id > 0;}
 private:
	GLenum ConvertGLType(BufferType bufferType) const;

	BufferType m_type;
	size_t m_size;
 	GLuint m_id;
};
