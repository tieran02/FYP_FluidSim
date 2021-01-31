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
