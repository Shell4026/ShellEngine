#include "Logger.h"

#include <iomanip>
#include <algorithm>
#include <sstream>
#include <thread>
#include <filesystem>

namespace sh::core
{
    Logger::Logger()
    {
        AddStream(std::cout);
    }
	bool Logger::AddStream(std::ostream& stream)
	{
        std::lock_guard<std::mutex> lock(mu);
        // 중복 추가 방지
        if (std::find(streams.begin(), streams.end(), &stream) == streams.end()) {
            streams.push_back(&stream);
            return true;
        }
        return false;
	}
    void Logger::RemoveStream(std::ostream& stream)
    {
        std::lock_guard<std::mutex> lock(mu);
        streams.erase(std::remove(streams.begin(), streams.end(), &stream), streams.end());
    }

    void Logger::Log(LogLevel level, std::string_view message, std::string_view name, int line)
    {
        std::lock_guard<std::mutex> lock(mu);

        std::string source{ std::filesystem::path{ std::string{name} }.filename().string() };

        std::string logMessage = "[" + GetTimestamp() + "][" + LevelToString(level) + "]";
        if (!name.empty())
            logMessage += fmt::format("[{}: {}]", source, line);
        logMessage += message.data();
        logMessage += '\n';

        for (auto& stream : streams) {
            (*stream) << logMessage;
            stream->flush();
        }
    }

    auto Logger::GetTimestamp() -> std::string 
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto in_time_t = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm buf;
#if defined(_WIN32) || defined(_WIN64)
        localtime_s(&buf, &in_time_t);
#else
        localtime_r(&in_time_t, &buf);
#endif

        std::ostringstream oss;
        oss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    auto Logger::LevelToString(LogLevel level) -> std::string
    {
        switch (level) 
        {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO ";
        case LogLevel::Warn:  return "WARN ";
        case LogLevel::Error: return "ERROR";
        default:              return "UNKNOWN";
        }
    }

    void Logger::Debug(std::string_view message, std::string_view name, int line) {
        Log(LogLevel::Debug, message, name, line);
    }

    void Logger::Info(std::string_view message, std::string_view name, int line) {
        Log(LogLevel::Info, message, name, line);
    }

    void Logger::Warn(std::string_view message, std::string_view name, int line) {
        Log(LogLevel::Warn, message, name, line);
    }

    void Logger::Error(std::string_view message, std::string_view name, int line) {
        Log(LogLevel::Error, message, name, line);
    }
}//namespace