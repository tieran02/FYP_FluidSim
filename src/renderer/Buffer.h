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

class Buffer : NonCopyable, NonMovable
{
 public:
	Buffer(BufferType bufferType);
	~Buffer();

	void Build(void* data, size_t size);
	void Bind() const;
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
