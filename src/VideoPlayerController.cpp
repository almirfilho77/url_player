#include <functional>
#include <iostream>

#include "VideoPlayerController.h"

// #define BIND_EVENT_FN(x) std::bind(&VideoPlayerController::x, std::placeholders::_1, this)

/* This function is called when an error message is posted on the bus */
static void error_cb(GstBus *bus, GstMessage *msg, void *data)
{
    VideoPlayerController *controller = static_cast<VideoPlayerController *>(data);
    GError *err;
    gchar *debug_info;

    /* Print error details on the screen */
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    /* Set the pipeline to READY (which stops playback) */
    gst_element_set_state(controller->GetPipeline(), GST_STATE_READY); // TODO: SYNC THIS WITH MODEL
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb(GstBus *bus, GstMessage *msg, void *data)
{
    VideoPlayerController *controller = static_cast<VideoPlayerController *>(data);
    auto model = controller->GetModel();
    
    g_print("End-Of-Stream reached.\n");
    gst_element_set_state(controller->GetPipeline(), GST_STATE_READY);
    model->SetState(GST_STATE_READY);
}

static void state_changed_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *data)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(data->GetPipeline()))
    {
        data->GetModel()->SetState(new_state);
        g_print("State set to %s\n", gst_element_state_get_name(new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
            /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
            // refresh_ui(data); // TODO: UNCOMMENT THIS OR CREATE A REFRESH UI FUNCTION IN VIEW
            std::cout << "Refresh UI\n";
        }
    }
}

VideoPlayerController::VideoPlayerController(const char *video_url, VideoPlayerModel *model, VideoPlayerView *view)
    :   m_bus(nullptr),
        m_videoPlayerModel(model),
        m_videoPlayerView(view),
        m_refresh_cb_handle(0)
{
    m_videoPlayerModel->SetVideoURL(video_url);
    if (!m_InitPipeline())
    {
        return;
    }
    m_InitBus();

    /* Register a function that GLib will call every second */
    m_refresh_cb_handle = g_timeout_add (500, (GSourceFunc)OnRefresh, this);
}

VideoPlayerController::~VideoPlayerController()
{
    Terminate();
}

void VideoPlayerController::Terminate()
{
    /* Do the clean-up */
    Stop();
    g_source_remove(m_refresh_cb_handle);
    gst_object_unref(m_playbin);
}

bool VideoPlayerController::m_InitPipeline()
{
    m_videoPlayerModel->SetDuration(GST_CLOCK_TIME_NONE);
    m_playbin = gst_element_factory_make("playbin", "playbin");
    if (!m_playbin)
    {
        g_printerr("Not all elements could be created.\n");
        return false;
    }
    /* Set URL to play */
    g_object_set(m_playbin, "uri", m_videoPlayerModel->GetVideoURL(), NULL);
    m_videoPlayerView->SetPipelineObject(m_playbin);
}

void VideoPlayerController::m_InitBus()
{
    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    m_bus = gst_element_get_bus(m_applicationData->playbin);
    gst_bus_add_signal_watch(m_bus);
    g_signal_connect (G_OBJECT (m_bus), "message::error", (GCallback)error_cb, this);
    g_signal_connect (G_OBJECT (m_bus), "message::state-changed", (GCallback)state_changed_cb, this);
    g_signal_connect (G_OBJECT (m_bus), "message::eos", (GCallback)eos_cb, this);
    gst_object_unref(m_bus);
}

VideoPlayerModel *VideoPlayerController::GetModel() const
{
    return m_videoPlayerModel;
}

VideoPlayerView *VideoPlayerController::GetView() const
{
    return m_videoPlayerView;
}

GstElement *VideoPlayerController::GetPipeline() const
{
    return m_playbin;
}

void VideoPlayerController::ConnectUISignals()
{
    m_videoPlayerView->SetDeleteCallback(&OnDelete, this);
    m_videoPlayerView->SetPlayCallback(&OnPlay, this);
    m_videoPlayerView->SetPauseCallback(&OnPause, this);
    m_videoPlayerView->SetPlayCallback(&OnStop, this);
}

void VideoPlayerController::Play()
{
    /* Start playing */
    GstStateChangeReturn ret;
    ret = gst_element_set_state(m_playbin, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("Unable to set the pipeline to the playing state.\n");
        gst_object_unref(m_playbin);
        Terminate();
    }
    else
    {
        m_videoPlayerModel->SetState(GST_STATE_PLAYING);
    }
}

void VideoPlayerController::Pause()
{
    GstStateChangeReturn ret;
    ret = gst_element_set_state(m_playbin, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("[!] Failed to set the pipeline to pause state");
        // How to treat these errors???
    }
    else
    {
        m_videoPlayerModel->SetState(GST_STATE_PAUSED);
    }
}

void VideoPlayerController::Stop()
{
    GstStateChangeReturn ret;
    ret = gst_element_set_state(m_playbin, GST_STATE_NULL);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        g_printerr("[!] Failed to set the pipeline to stop state");
        // How to treat these errors???
    }
    else
    {
        m_videoPlayerModel->SetState(GST_STATE_NULL);
    }
}

void VideoPlayerController::OnDelete(GtkWidget *widget, GdkEvent *event, void *data)
{
    OnStop(nullptr, data);
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->Terminate();
}

void VideoPlayerController::OnPlay(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->Play();
}

void VideoPlayerController::OnPause(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->Pause();
}

void VideoPlayerController::OnStop(GtkButton *button, void *data)
{
    auto videoPlayerController = static_cast<VideoPlayerController *>(data);
    videoPlayerController->Stop();
}

bool VideoPlayerController::OnRefresh(VideoPlayerController *controller)
{
    auto model = controller->GetModel();
    auto view = controller->GetView();
    
    /* If we didn't know it yet, query the stream duration */
    if (!GST_CLOCK_TIME_IS_VALID(model->GetDuration()))
    {
        gint64 duration;
        if (!gst_element_query_duration(controller->GetPipeline(), GST_FORMAT_TIME, &duration))
        {
            g_printerr("Could not query current duration.\n");
        }
        else
        {
            model->SetDuration(duration);
        }
    }

    gint64 current_time;
    if (gst_element_query_position(controller->GetPipeline(), GST_FORMAT_TIME, &current_time))
    {
        model->SetCurrentTime(current_time);
    }

    view->RefreshUI();
}
