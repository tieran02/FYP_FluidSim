#include "Camera.h"
#include <gtc/matrix_transform.hpp>
#include <platform/Window.h>

Camera::Camera(glm::vec3&& pos, float nearClip, float farClip, float fov) :
	m_position(std::move(pos)),
	m_nearClip(nearClip),
	m_farClip(farClip),
	m_fov(fov)
{
	m_perspective = calculatePerspective();
	m_view = glm::lookAt(m_position, glm::vec3(0,0,0), glm::vec3(0,1,0));
}

void Camera::LootAt(const glm::vec3& target)
{
	m_view = glm::lookAt(m_position, target, glm::vec3(0,1,0));
}

const glm::mat4& Camera::PerspectiveMatrix() const
{
	return m_perspective;
}

const glm::mat4& Camera::ViewMatrix() const
{
	return m_view;
}

glm::mat4 Camera::ViewPerspectiveMatrix() const
{
	return m_view * m_perspective;
}

glm::mat4 Camera::calculatePerspective() const
{
	float aspect = (float)Window::Width() / (float)Window::Height();
	return glm::perspective(m_fov,aspect,m_nearClip,m_farClip);
}

