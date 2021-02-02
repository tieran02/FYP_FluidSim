#pragma once

#include <glm.hpp>
#include <gtx/quaternion.hpp>

class Transform
{
 public:
	Transform();
	Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale);

	void SetPosition(const glm::vec3& position);
	void SetRotation(const glm::quat& rotation);
	void SetRotation(const glm::vec3& axis, float angle);
	void SetScale(const glm::vec3& scale);

	glm::vec3& Position();
	glm::vec3& Scale();
	glm::quat& Rotation();
	const glm::vec3& Position() const;
	const glm::vec3& Scale() const;
	const glm::quat& Rotation() const;

	glm::mat4 ModelMatrix() const;
 private:
	glm::vec3 m_position;
	glm::vec3 m_scale;
	glm::quat m_rotation;
};
