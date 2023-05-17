#include "VideoPlayerModel.h"

VideoPlayerModel::VideoPlayerModel()
{
    m_playerInfo.video_url = "";
    m_playerInfo.state = GST_STATE_NULL;
    m_playerInfo.current_time = 0;
    m_playerInfo.duration = 0;
}

VideoPlayerModel::~VideoPlayerModel()
{
    
}

const std::string &VideoPlayerModel::GetVideoURL() const
{
    return m_playerInfo.video_url;
}

void VideoPlayerModel::SetVideoURL(std::string &video_url)
{
    if (video_url.compare("") == 0)
    {
        return;
    }
    m_playerInfo.video_url = video_url;
    Notify();
}

void VideoPlayerModel::SetVideoURL(const char *video_url)
{
    if (strcmp(video_url, "") == 0)
    {
        return;
    }
    m_playerInfo.video_url = video_url;
    Notify();
}

const GstState VideoPlayerModel::GetState() const
{
    return m_playerInfo.state;
}

void VideoPlayerModel::SetState(GstState state)
{
    m_playerInfo.state = state;
    Notify();
}

const gint64 VideoPlayerModel::GetCurrentTime() const
{
    return m_playerInfo.current_time;
}

void VideoPlayerModel::SetCurrentTime(gint64 current_time)
{
    if (current_time > m_playerInfo.duration || current_time < 0)
    {
        return;
    }
    m_playerInfo.current_time = current_time;
    Notify();
}

const gint64 VideoPlayerModel::GetDuration() const
{
    return m_playerInfo.duration;
}

void VideoPlayerModel::SetDuration(gint64 duration)
{
    if (duration < 0)
    {
        return;
    }
    m_playerInfo.current_time = 0;
    m_playerInfo.duration = duration;
    Notify();
}

void VideoPlayerModel::Subscribe(ISubscriber *subscriber)
{
    if (subscriber == nullptr)
    {
        return;
    }
    m_subscribers.push_back(subscriber);
    Notify(subscriber);
}

void VideoPlayerModel::Unsubscribe(ISubscriber *subscriber)
{
    if (subscriber == nullptr)
    {
        return;
    }
    m_subscribers.remove(subscriber);
}

void VideoPlayerModel::Notify()
{
    for (auto subscriber : m_subscribers)
    {
        subscriber->Update(&m_playerInfo);
    }
}

void VideoPlayerModel::Notify(ISubscriber *subscriber)
{
    if (subscriber == nullptr)
    {
        return;
    }
    subscriber->Update(&m_playerInfo);
}