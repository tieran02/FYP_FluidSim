#include "stb_image.h"
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
	m_textureType = TextureType::TEXTURE2D;

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
	if(!data)
	{
		LOG_CORE_ERROR("Texture::CreateTextureFromFile failed to load at path:{0}", path);
		stbi_image_free(data);
		return;
	}

	m_width = width;
	m_height = height;
	m_textureType = TextureType::TEXTURE2D;

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

void Texture::CreateCubemapFromFile(const std::vector<std::string>& faces)
{
	if(m_textureID != 0)
		return;

	m_textureType = TextureType::CUBEMAP;

	glGenTextures(1, &m_textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			LOG_CORE_ERROR("Texture::CreateCubemapFromFile failed to load at path:{0}", faces[i]);
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::CopyTexture(const Texture& sourceTexture)
{
	if(m_textureID == 0 || sourceTexture.m_textureID == 0)
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

void Texture::Bind(GLuint activeTextureSlot) const
{
	activeTexture(activeTextureSlot);

	switch (m_textureType)
	{
	case TextureType::TEXTURE2D:
		glBindTexture(GL_TEXTURE_2D, m_textureID);
		break;
	case TextureType::CUBEMAP:
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
		break;
	}

}

void Texture::Unbind(GLuint activeTextureSlot) const
{
	activeTexture(activeTextureSlot);
	switch (m_textureType)
	{
	case TextureType::TEXTURE2D:
		glBindTexture(GL_TEXTURE_2D, 0);
		break;
	case TextureType::CUBEMAP:
		glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
		break;
	}
}

GLuint Texture::TextureID() const
{
	return m_textureID;
}

GLenum Texture::Format() const
{
	return m_format;
}
GLenum Texture::InteralForamt() const
{
	return m_internalFormat;
}
GLuint Texture::Width() const
{
	return m_width;
}
GLuint Texture::Height() const
{
	return m_height;
}

void Texture::activeTexture(GLuint activeTextureSlot) const
{
	if(activeTextureSlot >= 32)
		LOG_CORE_ERROR("Texture::activeTexture max active texture in OpenGL is 32 (31 index)");

	glActiveTexture(GL_TEXTURE0 + activeTextureSlot);
}
