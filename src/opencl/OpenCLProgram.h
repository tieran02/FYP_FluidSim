#pragma once
#define __CL_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <string>
#include <unordered_map>
#include <NonCopyable.h>
#include <NonMovable.h>

class OpenCLContext;

class OpenCLProgram : NonCopyable
{
 public:
	OpenCLProgram(const std::string& path, const OpenCLContext& context);
	~OpenCLProgram() = default;
	OpenCLProgram(OpenCLProgram&& other) noexcept;

	void AddKernel(const std::string& kernelName);
	const cl::Kernel* GetKernel(const std::string& kernelName) const;
 private:
	cl::Program m_program;
 	std::unordered_map<std::string, cl::Kernel> m_kernels;

 	void build(const std::string& path, const OpenCLContext& context);
};
