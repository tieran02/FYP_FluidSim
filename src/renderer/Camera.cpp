#include "Camera.h"
#include <gtc/matrix_transform.hpp>
#include <platform/Window.h>

Camera::Camera(const glm::vec3& pos, float nearClip, float farClip, float fov) :
	m_position(pos),
	m_nearClip(nearClip),
	m_farClip(farClip),
	m_fov(glm::radians(fov)),
	m_front(0.0f, 0.0f, -1.0f),
	m_up(0.0f, 1.0f, 0.0f),
	//lastX(Window::Width()/2.0f),
	//lastY(Window::Height()/2.0f),
	pitch(0.0f),
	yaw(0.0f)
{
	m_perspective = calculatePerspective();
	LootAt(m_position + m_up);
}

void Camera::LootAt(const glm::vec3& target)
{
	m_view = glm::lookAt(m_position, target, m_up);
}

void Camera::Move(const glm::vec3& amount)
{
	m_position += amount;
	LootAt(m_position + m_front);
}

void Camera::MousePosition(float xpos, float ypos)
{
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	m_front = glm::normalize(front);

	LootAt(m_position + m_front);
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

const glm::vec3& Camera::Forward() const
{
	return m_front;
}
const glm::vec3& Camera::Up() const
{
	return m_up;
}

glm::mat4 Camera::calculatePerspective() const
{
	float aspect = (float)Window::Width() / (float)Window::Height();
	return glm::perspective(m_fov,aspect,m_nearClip,m_farClip);
}


