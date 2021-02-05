#include "Buffer.h"
#include "Vertex.h"
#include <exception>
#include <util/Log.h>

Buffer::Buffer(BufferType bufferType) : m_type{bufferType}, m_id{0}
{

}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_id);
}

void Buffer::Build(void* data, size_t size, GLuint bindPoint)
{
	CORE_ASSERT(!m_id, "BufferID already set");

	if(m_id)
	{
		LOG_CORE_ERROR("Buffer already built, if you plan to set data use upload instead");
		return;
	}

	m_size = size;
	glGenBuffers(1, &m_id);

	GLenum glBufferType = ConvertGLType(m_type);
	glBindBuffer(glBufferType, m_id);
	glBufferData(glBufferType, size, data, GL_STATIC_DRAW);

	if(m_type == BufferType::STORAGE_BUFFER)
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindPoint, m_id);

	glBindBuffer(glBufferType, 0);
}

void Buffer::Bind(GLuint bindPoint) const
{
	if(m_type == BufferType::VERTEX_BUFFER)
		glBindVertexBuffer(bindPoint,m_id, 0, sizeof(Vertex));
	else
		glBindBuffer(ConvertGLType(m_type), m_id);
}

void Buffer::Unbind() const
{
	glBindBuffer(ConvertGLType(m_type), 0);
}

void Buffer::Upload(void* data, size_t size) const
{
	CORE_ASSERT(m_id, "Buffer not built");
	CORE_ASSERT(size <= m_size, "size is greater than Buffer size");

	GLenum glBufferType = ConvertGLType(m_type);
	glBindBuffer(glBufferType, m_id);
	glBufferSubData(glBufferType, 0, size, data);
	glBindBuffer(glBufferType, 0);

}

GLenum Buffer::ConvertGLType(BufferType bufferType) const
{
	switch (m_type)
	{
	case BufferType::VERTEX_BUFFER:
		return GL_ARRAY_BUFFER;
	case BufferType::ELEMENT_BUFFER:
		return GL_ELEMENT_ARRAY_BUFFER;
	case BufferType::UNIFORM_BUFFER:
		return GL_UNIFORM_BUFFER;
	case BufferType::STORAGE_BUFFER:
		return GL_SHADER_STORAGE_BUFFER;
	}

	return GL_INVALID_ENUM;
}

