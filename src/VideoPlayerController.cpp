#include <functional>
#include <iostream>

#include "VideoPlayerController.h"

// #define BIND_EVENT_FN(x) std::bind(&VideoPlayerController::x, std::placeholders::_1, this)

// Forward declaration of callback static functions
static void error_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller);
static void eos_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller);
static void state_changed_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller);

VideoPlayerController::VideoPlayerController(const char *video_url, VideoPlayerModel *model, VideoPlayerView *view)
    :   m_bus(nullptr),
        m_videoPlayerModel(model),
        m_videoPlayerView(view),
        m_refresh_cb_handle(0),
        m_playbin(nullptr)
{
    m_videoPlayerModel->SetVideoURL(video_url);
    if (!m_InitPipeline())
    {
        return;
    }
    m_InitBus();

    /* Register a function that GLib will call every second */
    m_refresh_cb_handle = g_timeout_add (500, (GSourceFunc)OnRefresh, this);
    _DEBUG("On Refresh callback handle: {0}", m_refresh_cb_handle);
}

VideoPlayerController::~VideoPlayerController()
{
    /* Do the clean-up */
    _DEBUG("Value of refcount for playbin = {0}", GST_OBJECT_REFCOUNT_VALUE(m_playbin));
    g_source_remove(m_refresh_cb_handle);
    gst_object_unref(m_playbin);
}

void VideoPlayerController::Terminate()
{
    gtk_main_quit();
}

bool VideoPlayerController::m_InitPipeline()
{
    _DEBUG("Init pipeline!\n");
    m_playbin = gst_element_factory_make("playbin", "playbin");
    if (!m_playbin)
    {
        _ERR("Not all elements could be created.\n");
        Terminate();
        return false;
    }
    /* Set URL to play */
    g_object_set(m_playbin, "uri", m_videoPlayerModel->GetVideoURL().c_str(), NULL);
    m_videoPlayerView->SetPipelineObject(m_playbin);
    return true;
}

void VideoPlayerController::m_InitBus()
{
    /* Instruct the bus to emit signals for each received message, and connect to the interesting signals */
    m_bus = gst_element_get_bus(m_playbin);
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

GstElement *VideoPlayerController::GetPipeline()
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
        _ERR("Unable to set the pipeline to the playing state.");
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
        _ERR("Failed to set the pipeline to pause state");
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
        _ERR("Failed to set the pipeline to stop state");
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

void VideoPlayerController::OnRefresh(VideoPlayerController *controller)
{
    _DEBUG("On Refresh!");

    /* We do not want to update anything unless we are in the PAUSED or PLAYING states */

    auto model = controller->GetModel();
    
    if (model->GetState() < GST_STATE_PAUSED)
    {
        _ERR("Won't refresh... state is {0}", GET_STATE_NAME(model->GetState()));
        return;
    }
    
    /* If we didn't know it yet, query the stream duration */
    gint64 duration;
    if (!GST_CLOCK_TIME_IS_VALID(model->GetDuration()))
    {
        GstElement *pipeline = controller->GetPipeline();
        if (!gst_element_query_duration(pipeline, GST_FORMAT_TIME, &duration))
        {
            _ERR("Could not query current duration.");
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

    auto view = controller->GetView();
    view->RefreshUI();
}

/* This function is called when an error message is posted on the bus */
static void error_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller)
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
    gst_element_set_state(controller->GetPipeline(), GST_STATE_READY); // TODO: SYNC THIS WITH MODEL
}

/* This function is called when an End-Of-Stream message is posted on the bus.
 * We just set the pipeline to READY (which stops playback) */
static void eos_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller)
{
    auto model = controller->GetModel();
    
    g_print("End-Of-Stream reached.\n");
    gst_element_set_state(controller->GetPipeline(), GST_STATE_READY);
    model->SetState(GST_STATE_READY);
}

static void state_changed_cb(GstBus *bus, GstMessage *msg, VideoPlayerController *controller)
{
    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(controller->GetPipeline()))
    {
        controller->GetModel()->SetState(new_state);
        g_print("State set to %s\n", gst_element_state_get_name(new_state));
        if (old_state == GST_STATE_READY && new_state == GST_STATE_PAUSED)
        {
            /* For extra responsiveness, we refresh the GUI as soon as we reach the PAUSED state */
            // refresh_ui(data); // TODO: UNCOMMENT THIS OR CREATE A REFRESH UI FUNCTION IN VIEW
            std::cout << "Refresh UI\n";
        }
    }
}