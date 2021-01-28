#pragma once
#include <cstdint>

class Renderer
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	void DrawFrame();
 private:
};
