#include "Logger.h"
#include "spdlog/async.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

std::shared_ptr<spdlog::logger> Logger::m_consoleLogger;
std::shared_ptr<spdlog::logger> Logger::m_fileLogger;
LoggerType Logger::m_activeLogger;

void Logger::Init(LoggerType type)
{
    spdlog::set_pattern("%^[%T] T:%t (%!): %n: %v%$");
    m_activeLogger = type;
    m_fileLogger = spdlog::basic_logger_st<spdlog::async_factory>("async_file_logger", "logs/async_log.txt");
    m_fileLogger->set_level(spdlog::level::trace);
    m_consoleLogger = spdlog::stdout_color_st("url_player_console_logger");
    m_consoleLogger->set_level(spdlog::level::trace);
}

std::shared_ptr<spdlog::logger>& Logger::GetLogger()
{
    switch(m_activeLogger)
    {
        case LoggerType::FILE:
            return Logger::GetFileLogger();

        case LoggerType::CONSOLE:
            return Logger::GetConsoleLogger();
        
        default:
            return Logger::GetConsoleLogger();
    }
}