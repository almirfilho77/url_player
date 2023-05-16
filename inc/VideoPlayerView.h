#ifndef __VIDEO_PLAYER_VIEW_H__
#define __VIDEO_PLAYER_VIEW_H__

#include <gst/video/videooverlay.h>
#include <gdk/gdk.h>
#if defined (GDK_WINDOWING_X11)
#include <gdk/gdkx.h>
#elif defined (GDK_WINDOWING_WIN32)
#include <gdk/gdkwin32.h>
#elif defined (GDK_WINDOWING_QUARTZ)
#include <gdk/gdkquartz.h>
#endif

#include "Common.h"

typedef struct _UIComponents
{
    GtkWidget *main_window;  /* The uppermost window, containing all other windows */
    GtkWidget *video_window; /* The drawing area where the video will be shown */
    GtkWidget *main_box;     /* VBox to hold main_hbox and the controls */
    GtkWidget *main_hbox;    /* HBox to hold the video_window and the stream info text widget */
    GtkWidget *controls;     /* HBox to hold the buttons and the slider */
    GtkWidget *play_button, *pause_button, *stop_button; /* Buttons */
    GtkWidget *slider;
} UIComponents;

class VideoPlayerView : public ISubscriber
{
public:
    VideoPlayerView(ApplicationData *data);
    ~VideoPlayerView();

    void SetApplicationData(ApplicationData *data);
    void CreateUI();

    void Update(void *data) override;

    void SetDeleteCallback(delete_callback_fn delete_cb, void *data);
    void SetPlayCallback(btn_callback_fn play_cb, void *data);
    void SetPauseCallback(btn_callback_fn pause_cb, void *data);
    void SetStopCallback(btn_callback_fn stop_cb, void *data);

private:
    ApplicationData *m_applicationData;
    PlayerInfo *m_playerInfo;
    UIComponents m_UIComponents;
};

#endif /* __VIDEO_PLAYER_VIEW_H__ */