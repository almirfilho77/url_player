#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <memory>

#include "spdlog/spdlog.h"

enum class LoggerType
{
    ALL = 0,
    FILE = 1,
    CONSOLE = 2
};

class Logger
{
public:
    static void Init(LoggerType type);

    static std::shared_ptr<spdlog::logger>& GetLogger();
    inline static std::shared_ptr<spdlog::logger>& GetConsoleLogger() { return m_consoleLogger; }
    inline static std::shared_ptr<spdlog::logger>& GetFileLogger() { return m_fileLogger; }

private:
    static std::shared_ptr<spdlog::logger> m_consoleLogger;
    static std::shared_ptr<spdlog::logger> m_fileLogger;
    static LoggerType m_activeLogger;
};

#define _DEBUG(...) Logger::GetLogger()->debug(__VA_ARGS__)
#define _INFO(...) Logger::GetLogger()->info(__VA_ARGS__)
#define _ERR(...) Logger::GetLogger()->error(__VA_ARGS__)

#endif /* __LOGGER_H__ */