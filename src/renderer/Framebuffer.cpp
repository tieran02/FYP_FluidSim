#include "Framebuffer.h"

#include "util/Log.h"

FrameBuffer::FrameBuffer(GLenum foramt, GLenum internalFormat) :m_format(foramt), m_internalFormat(internalFormat)
{
	
}

FrameBuffer::~FrameBuffer()
{
	if (m_textureID == 0)
		return;

	glDeleteRenderbuffers(1, &m_renderBufferObject);
	glDeleteTextures(1, &m_textureID);
	glDeleteFramebuffers(1, &m_frameBufferID);
}

FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
{
	m_frameBufferID = other.m_frameBufferID;
	m_textureID = other.m_textureID;
	m_renderBufferObject = other.m_renderBufferObject;
	m_width = other.m_width;
	m_height = other.m_height;
	m_format = other.m_format;
	m_internalFormat = other.m_internalFormat;
	
	other.m_frameBufferID = 0;
	other.m_textureID = 0;
	other.m_renderBufferObject = 0;
	other.m_width = 0;
	other.m_height = 0;
	other.m_format = 0;
	other.m_internalFormat = 0;
}

void FrameBuffer::Create(uint32_t width, uint32_t height)
{
	if (m_textureID != 0)
		return;

	m_width = width;
	m_height = height;
	
	glGenFramebuffers(1, &m_frameBufferID);
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);

	createTextureBuffer(width, height, m_format, m_internalFormat);

	// attach texture to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureID, 0);

	//Attach depth and stencil to framebuffer
	glGenRenderbuffers(1, &m_renderBufferObject);
	glBindRenderbuffer(GL_RENDERBUFFER, m_renderBufferObject);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_renderBufferObject);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
		LOG_CORE_ERROR("FRAMEBUFFER:: Framebuffer is not complete");
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferID);
}

void FrameBuffer::Unbind() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::createTextureBuffer(uint32_t width, uint32_t height, GLenum format, GLenum internalFormat)
{
	if(m_textureID != 0)
		return;
	
	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, nullptr);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
}
