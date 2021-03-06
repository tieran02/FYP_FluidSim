#pragma once
#include <vector>
#include "glm.hpp"
#include "opencl/OpenCLContext.h"

struct AABB;

class OpenCLHashmapTests
{
 public:
	struct hash_pair
	{
		int key;
		int neighbors[256];
		int pointsInBucket = 0;
	};
	
	OpenCLHashmapTests();
	~OpenCLHashmapTests() = default;

	void BuildTests(const std::vector<glm::vec4>& points, const AABB& aabb);
	void NearestNeighborTests(const std::vector<glm::vec4>& points, const AABB& aabb);
 private:
	OpenCLContext m_context;

	void buildProgram();
};
