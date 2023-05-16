#ifndef __EVENT_H__
#define __EVENT_H__

#include <string>

enum class EventType
{
    None = 0,
    Play, Pause, Stop, VideoChange,
};

class Event
{
public:
    virtual ~Event() = default;

    bool handled = false;

    virtual EventType GetEventType() const = 0;
    virtual const char *GetName() const = 0;
    virtual inline std::string ToString() const { return GetName(); }
};

#endif /* __EVENT_H__ */