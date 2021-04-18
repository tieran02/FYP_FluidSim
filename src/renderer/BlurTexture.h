#pragma once
#include <cstdint>

#include "glad/gl.h"

class Renderer;
class Mesh;
class Shader;

class BlurTexture
{
public:
	static void Blur(GLuint sourceTextureID, uint32_t textureWidth, uint32_t textureHeight, GLenum format, GLenum internalFormat, const Shader& blurShader, const Mesh& quadMesh, uint32_t amount);
};
