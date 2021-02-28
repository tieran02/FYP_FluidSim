#pragma once
#define __CL_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#endif

#include <NonCopyable.h>
#include <NonMovable.h>
#include "OpenCLProgram.h"


class OpenCLContext : NonCopyable, NonMovable
{
 public:
	OpenCLContext();
	~OpenCLContext() = default;

	const cl::Context& Context() const;
	const cl::Device& Device() const;

	void AddProgram(const std::string& name, const std::string& path);
	const OpenCLProgram* GetProgram(const std::string& name) const;
 private:
	cl::Context m_context;
	cl::Device m_device;
	cl::CommandQueue m_queue;

	std::unordered_map<std::string, OpenCLProgram> m_programs;
	void initilise();
};
