#pragma once

#include <platform/Window.h>
#include <cstdint>
#include <NonCopyable.h>
#include <renderer/primitives/Mesh.h>
#include <renderer/primitives/PlanePrimitive.h>
#include <renderer/primitives/SpherePrimitive.h>
#include "Shader.h"
#include "Buffer.h"

class Renderer : NonCopyable
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	~Renderer();
	void DrawFrame() const;
 private:
	void BuildVAO();

	unsigned int m_VAO;
	PlanePrimitive plane{glm::vec2(0.75f,0.5f)};
	SpherePrimitive sphere{0.5f,24,16};
	Shader shader;
};
