#pragma once

#include "Export.h"
#include "Singleton.hpp"
#include "SContainer.hpp"

#include "fmt/core.h"

#include <mutex>
#include <vector>
#include <iostream>
#include <string_view>

#define SH_INFO(message) sh::core::Logger::GetInstance()->Info(message, __func__)
#define SH_INFO_FORMAT(message, ...) sh::core::Logger::GetInstance()->Info(fmt::format(message, __VA_ARGS__), __func__)
#define SH_WARN(message) sh::core::Logger::GetInstance()->Warn(message, __func__)
#define SH_WARN(message, ...) sh::core::Logger::GetInstance()->Warn(fmt::format(message, __VA_ARGS__), __func__)
#define SH_ERROR(message) sh::core::Logger::GetInstance()->Error(message, __func__)
#define SH_ERROR(message, ...) sh::core::Logger::GetInstance()->Error(fmt::format(message, __VA_ARGS__), __func__)

namespace sh::core
{
	class Logger : public core::Singleton<Logger>
	{
		friend core::Singleton<Logger>;
	public:
		enum class LogLevel
		{
			Debug,
			Info,
			Warn,
			Error
		};

		SVector<std::ostream*> streams;
		std::mutex mu;
	protected:
		SH_CORE_API Logger();
	public:
		SH_CORE_API bool AddStream(std::ostream& stream);
		SH_CORE_API void RemoveStream(std::ostream& stream);

		SH_CORE_API auto GetTimestamp() -> std::string;
		SH_CORE_API auto LevelToString(LogLevel level) -> std::string;

		SH_CORE_API void Log(LogLevel level, std::string_view message, std::string_view name);

		SH_CORE_API void Debug(std::string_view message, std::string_view name);
		SH_CORE_API void Info(std::string_view message, std::string_view name);
		SH_CORE_API void Warn(std::string_view message, std::string_view name);
		SH_CORE_API void Error(std::string_view message, std::string_view name);
	};
}//namespace