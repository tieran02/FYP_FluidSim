#include "Transform.h"

Transform::Transform() :
	m_position(glm::vec3()),
	m_scale(glm::vec3(1)),
	m_rotation(glm::quat())
{

}

Transform::Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) :
	m_position(position),
	m_scale(m_scale),
	m_rotation(rotation)
{

}

void Transform::SetPosition(const glm::vec3& position)
{
	m_position = position;
}

void Transform::SetRotation(const glm::quat& rotation)
{
	m_rotation = rotation;
}

void Transform::SetScale(const glm::vec3& scale)
{
	m_scale = scale;
}

glm::vec3& Transform::Position()
{
	return m_position;
}

glm::vec3& Transform::Scale()
{
	return m_scale;
}

glm::quat& Transform::Rotation()
{
	return m_rotation;
}

const glm::vec3& Transform::Position() const
{
	return m_position;
}

const glm::vec3& Transform::Scale() const
{
	return m_scale;
}

const glm::quat& Transform::Rotation() const
{
	return m_rotation;
}

glm::mat4 Transform::ModelMatrix() const
{
	glm::mat4 modelMatrix = glm::mat4(1);

	modelMatrix = glm::translate(modelMatrix, m_position);
	modelMatrix = glm::scale(modelMatrix, m_scale);
	modelMatrix *=  glm::toMat4(m_rotation);

	return modelMatrix;
}
