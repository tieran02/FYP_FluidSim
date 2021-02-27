#include <util/Log.h>
#include "OpenCLContext.h"
#include "util/Util.h"

OpenCLContext::OpenCLContext()
{
	initilise();
}

const cl::Context& OpenCLContext::Context() const
{
	return m_context;
}
const cl::Device& OpenCLContext::Device() const
{
	return m_device;
}

void OpenCLContext::initilise()
{

	cl_int err = CL_SUCCESS;
	try
	{
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		cl_context_properties properties[] =
		{
			CL_CONTEXT_PLATFORM,
			(cl_context_properties)(platforms[0])(),
			0
		};
		if(platforms.empty())
		{
			LOG_CORE_FATAL("FAILED TO FIND A OPENCL PLATFORM");
			return;
		}

		LOG_CORE_INFO("OpenCL Platform: {0}- {1}- {2}- {3}", platforms[0].getInfo<CL_PLATFORM_PROFILE>(),
															platforms[0].getInfo<CL_PLATFORM_NAME>(),
															platforms[0].getInfo<CL_PLATFORM_VERSION>(),
															platforms[0].getInfo<CL_PLATFORM_VENDOR>());

		m_context = cl::Context(CL_DEVICE_TYPE_ALL, properties);
		std::vector<cl::Device> devices = m_context.getInfo<CL_CONTEXT_DEVICES>();

		if(devices.empty())
		{
			LOG_CORE_FATAL("FAILED TO FIND A OPENCL Device");
			return;
		}
		//Pick first device for now
		//TODO: should pick the best device if multiple
		m_device = devices[0];
		LOG_CORE_INFO("OpenCL Device: {0}", m_device.getInfo<CL_DEVICE_NAME>());

		m_queue = cl::CommandQueue(m_context,m_device,0,&err);
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("ERROR: {0} ({1})", err.what(), Util::GetCLErrorString(err.err()));

	}
}
