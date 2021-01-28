#pragma once
#include <cstdint>
#include "Shader.h"

class Renderer
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	~Renderer();
	void DrawFrame();
 private:
	unsigned int VBO, VAO;
	Shader shader;
};
