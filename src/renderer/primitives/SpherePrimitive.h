#pragma once

#include "Primitive.h"

class SpherePrimitive : public Primitive
{
 public:
	SpherePrimitive(float Radius, uint32_t LongitudeCount, uint32_t LatitudeCount);
	~SpherePrimitive() override;
	void Build() override;
 private:
	float m_radius;
	uint32_t m_longitudeCount, m_latitudeCount;
};
