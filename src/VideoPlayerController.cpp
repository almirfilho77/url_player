#include <functional>
#include <iostream>

#include "VideoPlayerController.h"

// #define BIND_EVENT_FN(x) std::bind(&VideoPlayerController::x, std::placeholders::_1, this)

/* This function is called when an error message is posted on the bus */
static void error_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *data)
{
    GError *err;
    gchar *debug_info;

    /* Print error details on the screen */
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    /* Set the pipeline to READY (which stops playback) */
    gst_element_set_state(data->GetAppData()->playbin, GST_STATE_READY); // TODO: SYNC THIS WITH MODEL
}

static void state_changed_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->GetAppData()->playbin))
    {
        data->GetAppData()->state = new_state; // TODO: SYNC THIS WITH MODEL
        g_print("State set to %s\n", gst_element_state_get_name(new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
            /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
            // refresh_ui(data); // TODO: UNCOMMENT THIS OR CREATE A REFRESH UI FUNCTION IN VIEW
            std::cout << "Refresh UI\n";
        }
    }
}

VideoPlayerController::VideoPlayerController(ApplicationData *app_data, VideoPlayerModel *model, VideoPlayerView *view)
    :   m_bus(nullptr),
        m_applicationData(app_data),
        m_videoPlayerModel(model),
        m_videoPlayerView(view)
{
    if (!m_InitPipeline())
    {
        return;
    }
}

VideoPlayerController::~VideoPlayerController()
{

}

bool VideoPlayerController::m_InitPipeline()
{
    m_applicationData->duration = GST_CLOCK_TIME_NONE;
    m_applicationData->playbin = gst_element_factory_make("playbin", "playbin");
    if (!m_applicationData->playbin)
    {
        g_printerr("Not all elements could be created.\n");
        return false;
    }
    /* Set URL to play */
    g_object_set(m_applicationData->playbin, "uri", m_applicationData->video_url, NULL);
}

void VideoPlayerController::m_InitBus()
{
    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    m_bus = gst_element_get_bus(m_applicationData->playbin);
    gst_bus_add_signal_watch(m_bus);
    g_signal_connect (G_OBJECT (m_bus), "message::error", (GCallback)error_cb, m_applicationData);
    g_signal_connect (G_OBJECT (m_bus), "message::state-changed", (GCallback)state_changed_cb, this);
    gst_object_unref(m_bus);
}

ApplicationData *VideoPlayerController::GetAppData() const
{
    return m_applicationData;
}

VideoPlayerModel *VideoPlayerController::GetModel() const
{
    return m_videoPlayerModel;
}

void VideoPlayerController::ConnectUISignals()
{
    m_videoPlayerView->SetDeleteCallback(&OnDelete, this);
    m_videoPlayerView->SetPlayCallback(&OnPlay, this);
    m_videoPlayerView->SetPauseCallback(&OnPause, this);
    m_videoPlayerView->SetPlayCallback(&OnStop, this);
}

void VideoPlayerController::OnDelete(GtkWidget *widget, GdkEvent *event, void *data)
{
    OnStop(nullptr, data);
    gtk_main_quit();
}

void VideoPlayerController::OnPlay(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->GetModel()->SetState(GST_STATE_PLAYING);
}

void VideoPlayerController::OnPause(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->GetModel()->SetState(GST_STATE_PAUSED);
}

void VideoPlayerController::OnStop(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->GetModel()->SetState(GST_STATE_NULL);
}