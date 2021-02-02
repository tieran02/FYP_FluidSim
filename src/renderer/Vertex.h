#pragma once
#include <glm.hpp>

struct Vertex
{
	Vertex();
	Vertex(glm::vec3 Position, glm::vec3 Normal, glm::vec2 TexCoord);
	//Copy constructor
	Vertex(const Vertex& other);
	Vertex& operator=(const Vertex& other);
	//Move constructor
	Vertex(Vertex&& other) noexcept;
	Vertex& operator=(Vertex&& other) noexcept;

	glm::vec3 Position{};
	glm::vec3 Normal{};
	glm::vec2 TexCoords{};

	static void EnableAttributes(int& attributeCount);
};
