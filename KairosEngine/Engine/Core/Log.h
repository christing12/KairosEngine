#pragma once
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/fmt/ostr.h"

namespace Kairos {
	class Log {
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return sCoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return sClientLogger; }
	private:
		// heap allocated & shared among multiple objects
		static std::shared_ptr<spdlog::logger> sCoreLogger;
		static std::shared_ptr<spdlog::logger> sClientLogger;
	};
}


// ::Kairos means namespace Kairos thats in the global namespace
#define KRS_CORE_TRACE(...)		::Kairos::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define KRS_CORE_INFO(...)		::Kairos::Log::GetCoreLogger()->info(__VA_ARGS__)
#define KRS_CORE_WARN(...)		::Kairos::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define KRS_CORE_ERROR(...)		::Kairos::Log::GetCoreLogger()->error(__VA_ARGS__)
#define KRS_CORE_FATAL(...)		::Kairos::Log::GetCoreLogger()->fatal(__VA_ARGS__)

#define KRS_TRACE(...)			::Kairos::Log::GetClientLogger()->trace(__VA_ARGS__)
#define KRS_INFO(...)			::Kairos::Log::GetClientLogger()->info(__VA_ARGS__)
#define KRS_WARN(...)			::Kairos::Log::GetClientLogger()->warn(__VA_ARGS__)
#define KRS_ERROR(...)			::Kairos::Log::GetClientLogger()->error(__VA_ARGS__)
#define KRS_FATAL(...)			::Kairos::Log::GetClientLogger()->fatal(__VA_ARGS__)

