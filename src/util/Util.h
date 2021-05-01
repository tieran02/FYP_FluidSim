#pragma once
#include <string>
#include <CL/cl.hpp>

class Util
{
 public:
	static std::string ReadFile(const std::string& path);

	static  const char* GetCLErrorString(cl_int error);

	static float MapValue(float inValue, float minInRange, float maxInRange, float minOutRange, float maxOutRange);
};

template <typename T>
inline T absmax(T x, T y) {
	return (x * x > y * y) ? x : y;
}
