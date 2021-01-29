#pragma once
#include <cstdint>
#include <NonCopyable.h>
#include "Shader.h"
#include "Buffer.h"

class Renderer : NonCopyable
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	~Renderer();
	void DrawFrame();
 private:
	unsigned int VAO;
	Buffer vertexBuffer{BufferType::VERTEX_BUFFER};
	Shader shader;
};
