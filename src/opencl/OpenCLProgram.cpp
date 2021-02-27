#include <util/Log.h>
#include "OpenCLProgram.h"
#include "OpenCLContext.h"
#include "util/Util.h"

OpenCLProgram::OpenCLProgram(const std::string& path, const OpenCLContext& context)
{
	build(path,context);
}

void OpenCLProgram::AddKernel(const std::string& kernelName)
{
	if(m_kernels.find(kernelName) != m_kernels.end())
		return;
	m_kernels[kernelName] = cl::Kernel(m_program, kernelName.c_str());
}

const cl::Kernel* OpenCLProgram::GetKernel(const std::string& kernelName) const
{
	if(m_kernels.find(kernelName) == m_kernels.end())
		return nullptr;
	return &m_kernels.at(kernelName);
}

void OpenCLProgram::build(const std::string& path, const OpenCLContext& context)
{
	std::string kernalStr = Util::ReadFile(path);
	cl::Program::Sources source(1, std::make_pair(kernalStr.data(), kernalStr.size()));
	std::vector<cl::Device> devices{context.Device()};

	try
	{
		m_program = cl::Program(context.Context(), source);
		m_program.build(devices);
	}
	catch (cl::Error& err)
	{
		if (err.err() == CL_BUILD_PROGRAM_FAILURE) {
			for (const cl::Device& dev : devices)
			{
				// Check the build status
				cl_build_status status = m_program.getBuildInfo<CL_PROGRAM_BUILD_STATUS>(dev);
				if (status != CL_BUILD_ERROR)
					continue;

				// Get the build log
				std::string name = dev.getInfo<CL_DEVICE_NAME>();
				std::string buildlog = m_program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev);
				LOG_CORE_ERROR("Kernel: {0}, build log for device {1} : {2}",path, name, buildlog);
			}
		}
		throw;
	}
}
