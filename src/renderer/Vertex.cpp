#include "Vertex.h"
#include "glad/gl.h"

void Vertex::EnableAttributes(int& attributeCount)
{
	// positions
	glEnableVertexAttribArray(attributeCount);
	glVertexAttribFormat(attributeCount, 3, GL_FLOAT, false, offsetof(Vertex, Position));
	glVertexAttribDivisor(attributeCount, 0);
	glVertexAttribBinding(attributeCount++, 0);

	// normals
	glEnableVertexAttribArray(attributeCount);
	glVertexAttribFormat(attributeCount, 3, GL_FLOAT, false, offsetof(Vertex, Normal));
	glVertexAttribDivisor(attributeCount, 0);
	glVertexAttribBinding(attributeCount++, 0);

	// texture coords
	glEnableVertexAttribArray(attributeCount);
	glVertexAttribFormat(attributeCount, 2, GL_FLOAT, false, offsetof(Vertex, TexCoords));
	glVertexAttribDivisor(attributeCount, 0);
	glVertexAttribBinding(attributeCount++, 0);
}

Vertex::Vertex() : Position{0.0f,0.0f,0.0f}, Normal{0.0f,0.0f,0.0f}, TexCoords{0.0f,0.0f}
{

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

