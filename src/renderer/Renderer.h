#pragma once

#include <platform/Window.h>
#include <cstdint>
#include <NonCopyable.h>
#include <renderer/primitives/Mesh.h>
#include "Shader.h"
#include "Buffer.h"

class Renderer : NonCopyable
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	~Renderer();
	void DrawFrame();
 private:
	void BuildVAO();

	unsigned int m_VAO;
	Buffer vertexBuffer1{BufferType::VERTEX_BUFFER};
	Mesh mesh;
	Shader shader;
};
