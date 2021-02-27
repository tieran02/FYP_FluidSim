#pragma once
#define __CL_ENABLE_EXCEPTIONS
#define CL_TARGET_OPENCL_VERSION 300
#if defined(__APPLE__) || defined(__MACOSX)
#include <OpenCL/cl.hpp>
#else
#include <CL/cl.hpp>
#include <NonCopyable.h>
#include <NonMovable.h>
#endif

class OpenCLContext : NonCopyable, NonMovable
{
 public:
	OpenCLContext();
	~OpenCLContext() = default;

 private:
	cl::Context m_context;
	cl::Device m_device;
	cl::CommandQueue m_queue;

	void initilise();
};
