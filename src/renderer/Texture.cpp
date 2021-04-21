#include <util/Log.h>
#include "Texture.h"
#include "Shader.h"
#include "primitives/Mesh.h"
#include "Framebuffer.h"

Texture::Texture(GLenum format, GLenum internalFormat) : m_format(format), m_internalFormat(format)
{

}

Texture::~Texture()
{
	if(m_textureID != 0)
		glDeleteTextures(1, &m_textureID);
}

Texture::Texture(Texture&& other) noexcept
{
	m_textureID = other.m_textureID;
	m_format = other.m_format;
	m_internalFormat = other.m_internalFormat;
	m_width = other.m_width;
	m_height = other.m_height;

	other.m_textureID = 0;
	other.m_format = 0;
	other.m_internalFormat = 0;
	other.m_width = 0;
	other.m_height = 0;
}

void Texture::CreateEmptyTexture2D(GLuint width, GLuint height)
{
	if(m_textureID != 0)
		return;

	m_width = width;
	m_height = height;

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0, m_format, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::CreateTextureFromFile(const std::string& path)
{
	if(m_textureID != 0)
		return;

	int width, height, nrChannels;
	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);

	m_width = width;
	m_height = height;

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_2D, m_textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, width, height, 0, m_format, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(data);
}

void Texture::CopyTexture(const Texture& sourceTexture)
{
	if(m_textureID != 0 || sourceTexture.m_textureID == 0)
		return;

	if(m_format != sourceTexture.m_format)
	{
		LOG_CORE_FATAL("Texture::CopyTexture source format is not the same as texture format");
		return;
	}
	if(m_internalFormat != sourceTexture.m_internalFormat)
	{
		LOG_CORE_FATAL("Texture::CopyTexture source internal format is not the same as texture internal format");
		return;
	}

	CreateEmptyTexture2D(sourceTexture.m_width, sourceTexture.m_height);

	glCopyImageSubData(sourceTexture.m_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_width, m_height, 1);
}

void Texture::BlurTexture(const Shader& blurShader, const Mesh& quadMesh, uint32_t amount)
{
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, m_internalFormat, m_width, m_height, 0, m_format, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);

		// also check if framebuffers are complete (no need for depth buffer)
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			LOG_CORE_ERROR("BlurTexture::Framebuffer not complete!");
	}

	//now blur
	bool horizontal = true, first_iteration = true;
	blurShader.Bind();
	for (unsigned int i = 0; i < amount; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
		blurShader.SetInt("horizontal", horizontal);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, first_iteration ? m_textureID : pingpongBuffer[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)

		quadMesh.Draw();
		horizontal = !horizontal;
		if (first_iteration)
			first_iteration = false;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	blurShader.Unbind();

	glCopyImageSubData(pingpongBuffer[!horizontal], GL_TEXTURE_2D, 0, 0, 0, 0,
		m_textureID, GL_TEXTURE_2D, 0, 0, 0, 0,
		m_width, m_height, 1);

	//destroy
	glDeleteTextures(2, &pingpongBuffer[0]);
	glDeleteFramebuffers(2, &pingpongFBO[0]);
}
