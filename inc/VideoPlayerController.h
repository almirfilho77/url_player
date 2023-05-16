#ifndef __VIDEO_PLAYER_CONTROLLER_H__
#define __VIDEO_PLAYER_CONTROLLER_H__

#include "Common.h"
#include "VideoPlayerModel.h"
#include "VideoPlayerView.h"

class VideoPlayerController
{
public:
    VideoPlayerController(ApplicationData *data, VideoPlayerModel *model, VideoPlayerView *view);
    ~VideoPlayerController();

    void SetApplicationData(ApplicationData *data);

    ApplicationData *GetAppData() const;
    VideoPlayerModel *GetModel() const;
    void ConnectUISignals();

    static void OnDelete(GtkWidget *widget, GdkEvent *event, void *data);
    static void OnPlay(GtkButton *button, void *data);
    static void OnPause(GtkButton *button, void *data);
    static void OnStop(GtkButton *button, void *data);

private:
    bool m_InitPipeline();
    void m_InitBus();

    GstBus *m_bus;
    ApplicationData *m_applicationData;
    VideoPlayerModel *m_videoPlayerModel;
    VideoPlayerView *m_videoPlayerView;
};

#endif /* __VIDEO_PLAYER_CONTROLLER_H__ */