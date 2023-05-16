#ifndef __VIDEO_EVENT_H__
#define __VIDEO_EVENT_H__

#include <string>

#include "Event.h"

class VideoPlayerEvent : public Event
{
public:
    virtual ~VideoPlayerEvent();


};

class PlayEvent : public VideoPlayerEvent
{
    
};

class VideoChangeEvent : public VideoPlayerEvent
{

};

#endif /* __VIDEO_EVENT_H__ */