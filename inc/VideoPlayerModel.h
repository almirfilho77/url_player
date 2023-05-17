#ifndef __VIDEO_PLAYER_MODEL_H__
#define __VIDEO_PLAYER_MODEL_H__

#include <list>

#include "Common.h"
#include "ISubject.h"

class VideoPlayerModel : public ISubject
{
public:
    VideoPlayerModel();
    ~VideoPlayerModel();

    void Subscribe(ISubscriber *subscriber) override;
    void Unsubscribe(ISubscriber *subscriber) override;
    void Notify() override;
    void Notify(ISubscriber *subscriber) override;

    const std::string &GetVideoURL() const;
    void SetVideoURL(std::string &video_url);
    void SetVideoURL(const char *video_url);

    const GstState GetState() const;
    void SetState(GstState state);

    const gint64 GetCurrentTime() const;
    void SetCurrentTime(gint64 current_time);

    const gint64 GetDuration() const;
    void SetDuration(gint64 duration);

private:
    PlayerInfo m_playerInfo;
    std::list<ISubscriber *> m_subscribers;
};

#endif /* __VIDEO_PLAYER_MODEL_H__ */