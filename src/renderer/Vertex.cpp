#include "Vertex.h"
#include "glad/glad.h"

void Vertex::EnableAttributes()
{
	// positions
	glEnableVertexAttribArray(0);
	glVertexAttribFormat(0, 3, GL_FLOAT, false, offsetof(Vertex, Position));
	glVertexAttribBinding(0, 0);

	// normals
	glEnableVertexAttribArray(1);
	glVertexAttribFormat(1, 3, GL_FLOAT, false, offsetof(Vertex, Normal));
	glVertexAttribBinding(1, 0);
	// texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribFormat(2, 2, GL_FLOAT, false, offsetof(Vertex, TexCoords));
	glVertexAttribBinding(2, 0);
}

Vertex::Vertex(glm::vec3 Position, glm::vec3 Normal, glm::vec2 TexCoord)
{
	this->Position = Position;
	this->Normal = Normal;
	this->TexCoords = TexCoord;
}

Vertex::Vertex(const Vertex& other)
{
	Position = other.Position;
	Normal = other.Normal;
	TexCoords = other.TexCoords;
}

Vertex& Vertex::operator=(const Vertex& other)
{
	if (this != &other) // not a self-assignment
	{
		Position = other.Position;
		Normal = other.Normal;
		TexCoords = other.TexCoords;
	}

	return *this;
}

Vertex::Vertex(Vertex&& other) noexcept
{
	Position = std::move(other.Position);
	Normal = std::move(other.Normal);
	TexCoords = std::move(other.TexCoords);
}

Vertex& Vertex::operator=(Vertex&& other) noexcept
{
	if (this != &other) // not a self-assignment
	{
		Position = std::move(other.Position);
		Normal = std::move(other.Normal);
		TexCoords = std::move(other.TexCoords);
	}

	return *this;
}
