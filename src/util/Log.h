#pragma once

#include <NonMovable.h>
#include <NonCopyable.h>
#include <memory>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/basic_file_sink.h"

class Log : NonCopyable, NonMovable
{
 public:
	static void Init();
	static void Clean();
	static std::shared_ptr<spdlog::logger> GetCoreLogger();
 private:
	static std::shared_ptr<spdlog::logger> s_CombinedLogger;
};

#define LOG_CORE_TRACE(...) ::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define LOG_CORE_INFO(...) ::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_WARNING(...) ::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_ERROR(...) ::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_FATAL(...) ::Log::GetCoreLogger()->critical(__VA_ARGS__); __debugbreak()

#ifdef CORE_DEBUG
#define CORE_ASSERT(x, ...) { if(!(x)) { LOG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); assert(x); } }
#else
#define CORE_ASSERT(x, ...)
#endif // ASSERTIONS