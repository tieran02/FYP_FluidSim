#include "SpherePrimitive.h"
#include "gtc/constants.hpp"


SpherePrimitive::SpherePrimitive(float Radius, uint32_t LongitudeCount, uint32_t LatitudeCount) : m_radius(Radius),
	m_longitudeCount(LongitudeCount),
	m_latitudeCount(LatitudeCount)
{

}

SpherePrimitive::~SpherePrimitive()
{

}

void SpherePrimitive::Build()
{
	constexpr auto pi = glm::pi<float>();
	constexpr auto pi2 = glm::two_pi<float>();

	std::vector<Vertex> verts((m_longitudeCount+1) * m_latitudeCount + 2);
	verts[0].Position = glm::vec3(0.0f, 1.0f, 0.0f) * m_radius;
	verts[0].Normal = glm::normalize(verts[0].Position);
	verts[0].TexCoords = glm::vec2(0.0f, 0.0f);

	for (int lat = 0; lat < m_latitudeCount; lat++)
	{
		float a1 = pi * (float)(lat+1) / (float)(m_latitudeCount+1);
		float sin1 = sin(a1);
		float cos1 = cos(a1);

		for (int lon = 0; lon  <= m_longitudeCount; lon++)
		{
			float a2 = pi2 * (float)(lon == m_longitudeCount ? 0 : lon) / m_longitudeCount;
			float sin2 = sin(a2);
			float cos2 = cos(a2);

			glm::vec3 pos = glm::vec3(sin1 * cos2, cos1, sin1 * sin2 ) * m_radius;
			verts[lon+lat * (m_longitudeCount + 1) + 1].Position = pos;
			verts[lon+lat * (m_longitudeCount + 1) + 1].Normal = glm::normalize(pos);
			verts[lon+lat * (m_longitudeCount + 1) + 1].TexCoords = glm::vec2((float)lon / m_longitudeCount, 1.0f - (float)(lat+1) / (float)(m_latitudeCount+1));
		}
	}
	verts[verts.size()-1].Position = glm::vec3(0.0f, 1.0f, 0.0f) * -m_radius;
	verts[verts.size()-1].Normal = glm::normalize(verts[verts.size()-1].Position);
	verts[verts.size()-1].TexCoords = glm::vec2((float)m_longitudeCount, 1.0f - (float)(m_latitudeCount+1) / (float)(m_latitudeCount+1));

	//indices
	int numFaces = verts.size();
	int numTriangles = numFaces * 2;
	int numIndexes = numTriangles * 3;
	std::vector<uint32_t> indices(numIndexes);

	int i = 0;
	//top part of the sphere
	for (int lon = 0; lon < m_longitudeCount; lon++)
	{
		indices[i++] = lon+2;
		indices[i++] = lon+1;
		indices[i++] = 0;
	}

	for( int lat = 0; lat < m_latitudeCount - 1; lat++ )
	{
		for( int lon = 0; lon < m_longitudeCount; lon++ )
		{
			int current = lon + lat * (m_longitudeCount + 1) + 1;
			int next = current + m_longitudeCount + 1;

			indices[i++] = current;
			indices[i++] = current + 1;
			indices[i++] = next + 1;

			indices[i++] = current;
			indices[i++] = next + 1;
			indices[i++] = next;
		}
	}

	//Bottom part
	for( int lon = 0; lon < m_longitudeCount; lon++ )
	{
		indices[i++] = verts.size() - 1;
		indices[i++] = verts.size() - (lon+2) - 1;
		indices[i++] = verts.size() - (lon+1) - 1;
	}

	m_mesh.Build(std::move(verts), std::move(indices));
}
