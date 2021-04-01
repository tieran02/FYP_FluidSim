#include "Log.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/basic_file_sink.h"

std::shared_ptr<spdlog::logger> Log::s_CombinedLogger = nullptr;

void Log::Init()
{
	spdlog::set_pattern("%^[%T} %n: %v%$");
	std::vector<spdlog::sink_ptr> sinks;
	sinks.emplace_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
	sinks.emplace_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/log.txt"));
	s_CombinedLogger = std::make_shared<spdlog::logger>("LOG", sinks.begin(), sinks.end());
	#ifdef CORE_DEBUG
		s_CombinedLogger->set_level(spdlog::level::trace);
	#else
		s_CombinedLogger->set_level(spdlog::level::info);
	#endif
}

void Log::Clean()
{
	s_CombinedLogger.reset(); 
}

std::shared_ptr<spdlog::logger> Log::GetCoreLogger()
{
	assert(s_CombinedLogger);
	return s_CombinedLogger;
}