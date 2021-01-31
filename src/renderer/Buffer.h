#pragma once
#include <glad/glad.h>
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
	void Bind();
	void Unbind();

	GLuint ID() const {return m_id;}
	bool Valid() const {return m_id > 0;}
 private:
	BufferType m_type;
 	GLuint m_id;
};
