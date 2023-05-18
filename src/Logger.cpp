#include "spdlog/sinks/stdout_color_sinks.h"

#include "Logger.h"

std::shared_ptr<spdlog::logger> Logger::m_logger;

void Logger::Init()
{
    spdlog::set_pattern("%^[%T] %n: %v%$");
    m_logger = spdlog::stdout_color_mt("url_player_logger");
    m_logger->set_level(spdlog::level::level_enum::trace);
}