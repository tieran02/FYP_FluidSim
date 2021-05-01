#pragma once

#include <platform/Window.h>
#include <cstdint>
#include <NonCopyable.h>
#include <renderer/primitives/Mesh.h>
#include <renderer/primitives/QuadPrimitive.h>
#include "Shader.h"
#include "Buffer.h"

class Renderer : NonCopyable, NonMovable
{
 public:
	Renderer(uint32_t viewportWidth, uint32_t viewportHeight);
	virtual ~Renderer();

	void BeginFrame() const;
	void EndFrame() const;
	void Clear() const;
	void ClearColor(const glm::vec3& color = glm::vec3(0.0f)) const;

	void Draw(const Mesh& mesh, const Shader& shader, const Transform& transform) const;
	void Draw(const Mesh& mesh, const Shader& shader) const;
	void DrawInstanced(const Mesh& mesh, const Shader& shader, const Buffer& instanceBuffer, size_t instanceCount) const;
	void DrawInstanced(const Mesh& mesh, const Shader& shader, const std::vector<Buffer>& instanceBuffer, size_t instanceCount) const;
 private:
	void BuildVAO();
 protected:
	unsigned int m_VAO;
};
