#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <memory>

#include "spdlog/spdlog.h"

class Logger
{
public:
    static void Init();

    inline static std::shared_ptr<spdlog::logger>& GetLogger() { return m_logger; }

private:
    static std::shared_ptr<spdlog::logger> m_logger;
};

#define _DEBUG(...) Logger::GetLogger()->debug(__VA_ARGS__)
#define _INFO(...) Logger::GetLogger()->info(__VA_ARGS__)
#define _ERR(...) Logger::GetLogger()->error(__VA_ARGS__)

#endif /* __LOGGER_H__ */