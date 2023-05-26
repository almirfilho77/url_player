#ifndef __VIDEO_PLAYER_CONTROLLER_H__
#define __VIDEO_PLAYER_CONTROLLER_H__

#include "Common.h"
#include "VideoPlayerModel.h"
#include "VideoPlayerView.h"

class VideoPlayerController
{
public:
    VideoPlayerController(const char *video_url, VideoPlayerModel *model, VideoPlayerView *view);
    ~VideoPlayerController();
    void Terminate();

    VideoPlayerModel *GetModel() const;
    VideoPlayerView *GetView() const;
    GstElement *GetPipeline();

    void ConnectUISignals();

    void Play();
    void Pause();
    void Stop();

    static void OnDelete(GtkWidget *widget, GdkEvent *event, void *data);
    static void OnPlay(GtkButton *button, void *data);
    static void OnPause(GtkButton *button, void *data);
    static void OnRefresh(VideoPlayerController *controller);
    static void OnStop(GtkButton *button, void *data);

private:
    bool m_InitPipeline();
    void m_InitBus();

    guint m_refresh_cb_handle;
    GstBus *m_bus;
    GstElement *m_playbin;
    ApplicationData *m_applicationData;
    VideoPlayerModel *m_videoPlayerModel;
    VideoPlayerView *m_videoPlayerView;
};

#endif /* __VIDEO_PLAYER_CONTROLLER_H__ */