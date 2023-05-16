#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "Common.h"
#include "VideoPlayerController.h"
#include "VideoPlayerModel.h"
#include "VideoPlayerView.h"

class Application
{
public:
    Application(const char *video_url);
    ~Application();

private:
    ApplicationData *m_appData;
    VideoPlayerView *m_videoPlayerView;
    VideoPlayerModel *m_videoPlayerModel;
    VideoPlayerController *m_videoPlayerController;
};

#endif /* __APPLICATION_H__ */