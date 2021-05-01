#include <util/Log.h>
#include "OpenCLContext.h"
#include "util/Util.h"

#include "platform/Window.h"


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

const cl::CommandQueue& OpenCLContext::Queue() const
{
	return m_queue;
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
			CL_GL_CONTEXT_KHR, (cl_context_properties)glfwGetWGLContext(Window::GetGLFWwindow()),
			CL_WGL_HDC_KHR, (cl_context_properties)GetDC(glfwGetWin32Window(Window::GetGLFWwindow())),
			CL_CONTEXT_PLATFORM, (cl_context_properties)(platforms[0])(),
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

		m_context = cl::Context(CL_DEVICE_TYPE_GPU, properties);
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

		/*if (m_device.getInfo<CL_DEVICE_EXTENSIONS>().find("cl_khr_gl_sharing") == std::string::npos)
			LOG_CORE_FATAL("OpenCL context doesn't have gl sharing extension");*/

		m_queue = cl::CommandQueue(m_context,m_device,0,&err);
	}
	catch (cl::Error& err)
	{
		LOG_CORE_ERROR("ERROR: {0} ({1})", err.what(), Util::GetCLErrorString(err.err()));

	}
}

void OpenCLContext::AddProgram(const std::string& name, const std::string& path)
{
	if(m_programs.find(name) != m_programs.end())
		return;

	m_programs.emplace(name, OpenCLProgram(path,*this));
}

const OpenCLProgram* OpenCLContext::GetProgram(const std::string& name) const
{
	if(m_programs.find(name) == m_programs.end())
		return nullptr;

	return &m_programs.at(name);
}

OpenCLProgram* OpenCLContext::GetProgram(const std::string& name)
{
	if(m_programs.find(name) == m_programs.end())
		return nullptr;

	return &m_programs.at(name);
}
