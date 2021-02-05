#pragma once

#include <platform/Window.h>
#include <cstdint>
#include <NonCopyable.h>
#include <renderer/primitives/Mesh.h>
#include <renderer/primitives/QuadPrimitive.h>
#include <renderer/primitives/SpherePrimitive.h>
#include <array>
#include "Shader.h"
#include "Buffer.h"
#include "Camera.h"

class Renderer : NonCopyable, NonMovable
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	~Renderer();

	void BeginFrame() const;
	void EndFrame() const;

	void Draw(const Mesh& mesh, const Shader& shader, const Transform& transform) const;
	void DrawInstanced(const Mesh& mesh, const Shader& shader, const Buffer& instanceBuffer, size_t instanceCount) const;
 private:
	void BuildVAO();

	unsigned int m_VAO;
};
