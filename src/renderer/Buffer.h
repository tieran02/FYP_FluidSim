#pragma once
#include <glad/glad.h>
#include <NonCopyable.h>

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

	void Build(void* data, size_t size);
	GLuint ID() const {return m_id;}
 private:
	BufferType m_type;
 	GLuint m_id;
};
