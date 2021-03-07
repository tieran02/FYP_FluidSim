#pragma once
#include <opencl/OpenCLContext.h>
#include <vector>
#include <glm.hpp>



class OpenCLBruteForceNNTests
{
 public:
	OpenCLBruteForceNNTests();
	void BuildTests(const std::vector<glm::vec4>& points);
 private:
	OpenCLContext m_context;

	void buildProgram();
};
