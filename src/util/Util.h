#pragma once
#include <string>
#include <CL/cl.hpp>

class Util
{
 public:
	static std::string ReadFile(const std::string& path);

	static  const char* GetCLErrorString(cl_int error);
};
