#pragma once

#include <glm.hpp>

class Camera
{
 public:
	Camera(glm::vec3&& pos, float nearClip, float farClip, float fov);
	void LootAt(const glm::vec3& target);

	const glm::mat4& PerspectiveMatrix() const;
	const glm::mat4& ViewMatrix() const;
	glm::mat4 ViewPerspectiveMatrix() const;
 private:
	glm::vec3 m_position;
 	float m_nearClip, m_farClip, m_fov;

	glm::mat4 calculatePerspective() const;
 	glm::mat4 m_perspective, m_view;
};
