#include "BlurTexture.h"
#include "Renderer.h"
#include "Shader.h"
#include "primitives/Mesh.h"
#include "util/Log.h"

void BlurTexture::Blur(GLuint sourceTextureID, uint32_t textureWidth, uint32_t textureHeight, GLenum format, GLenum internalFormat, const Shader& blurShader, const Mesh& quadMesh, uint32_t amount)
{
    Blur(sourceTextureID, sourceTextureID, textureWidth, textureHeight, format, internalFormat, blurShader, quadMesh, amount);
}

void BlurTexture::Blur(GLuint sourceTextureID, GLuint destTextureID, uint32_t textureWidth, uint32_t textureHeight,
	GLenum format, GLenum internalFormat, const Shader& blurShader, const Mesh& quadMesh, uint32_t amount)
{
    unsigned int pingpongFBO[2];
    unsigned int pingpongBuffer[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, textureWidth, textureHeight, 0, format, GL_FLOAT, NULL);
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
        glBindTexture(GL_TEXTURE_2D, first_iteration ? sourceTextureID : pingpongBuffer[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)

        quadMesh.Draw();
        horizontal = !horizontal;
        if (first_iteration)
            first_iteration = false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    blurShader.Unbind();

    glCopyImageSubData(pingpongBuffer[!horizontal], GL_TEXTURE_2D, 0, 0, 0, 0,
        destTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,
        textureWidth, textureHeight, 1);

    //destroy
    glDeleteTextures(2, &pingpongBuffer[0]);
    glDeleteFramebuffers(2, &pingpongFBO[0]);
}
