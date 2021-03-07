#pragma once

#include <opencl/OpenCLContext.h>
#include "vec3.hpp"

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

	void InsertTests(const std::vector<glm::vec3>& points);

 private:
	OpenCLContext m_context;

	void compileProgram();
};
