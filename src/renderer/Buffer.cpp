#include "Buffer.h"
#include "Vertex.h"
#include <cassert>
#include <exception>

Buffer::Buffer(BufferType bufferType) : m_type{bufferType}, m_id{0}
{

}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_id);
}

void Buffer::Build(void* data, size_t size)
{
	assert(!m_id); //check if buffer already exists

	glGenBuffers(1, &m_id);

	switch (m_type)
	{
		case BufferType::VERTEX_BUFFER:
			glBindBuffer(GL_ARRAY_BUFFER, m_id);
			glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			break;
		case BufferType::ELEMENT_BUFFER:
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			break;
		case BufferType::UNIFORM_BUFFER:
			throw std::exception("UNIFORM_BUFFER is not implemented yet.");
		case BufferType::STORAGE_BUFFER:
			throw std::exception("STORAGE_BUFFER is not implemented yet.");
	}
}

void Buffer::Bind()
{
	switch (m_type)
	{
	case BufferType::VERTEX_BUFFER:
		// use glBindVertexBuffer as we are using separate_attrib_format, decoupled vertex layout from VBO
		glBindVertexBuffer(0,m_id, 0, sizeof(Vertex));
		break;
	case BufferType::ELEMENT_BUFFER:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
		break;
	case BufferType::UNIFORM_BUFFER:
		throw std::exception("UNIFORM_BUFFER is not implemented yet.");
	case BufferType::STORAGE_BUFFER:
		throw std::exception("STORAGE_BUFFER is not implemented yet.");
	}
}

void Buffer::Unbind()
{
	switch (m_type)
	{
	case BufferType::VERTEX_BUFFER:
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		break;
	case BufferType::ELEMENT_BUFFER:
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		break;
	case BufferType::UNIFORM_BUFFER:
		throw std::exception("UNIFORM_BUFFER is not implemented yet.");
	case BufferType::STORAGE_BUFFER:
		throw std::exception("STORAGE_BUFFER is not implemented yet.");
	}
}
