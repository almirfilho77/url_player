#include <iomanip>
#include <iostream>

#include "VideoPlayerView.h"

#define GET_STATE_NAME(x) ( (x == GST_STATE_VOID_PENDING) ? "VOID PENDING" : \
                            (x == GST_STATE_NULL) ? "NULL" : \
                            (x == GST_STATE_READY) ? "READY" : \
                            (x == GST_STATE_PAUSED) ? "PAUSED" : \
                            (x == GST_STATE_PLAYING) ? "PLAYING" : "UNKNOWN")

/* This function is called when the GUI toolkit creates the physical window that will hold the video.
 * At this point we can retrieve its handler (which has a different meaning depending on the windowing system)
 * and pass it to GStreamer through the VideoOverlay interface. */
static void realize_cb(GtkWidget *widget, void *data)
{
    std::cout << "[!] Realized\n";
    GstElement *pipeline = static_cast<GstElement *>(data);

    // This does not work in gtk-4
    GdkWindow *window = gtk_widget_get_window(widget);
    guintptr window_handle;

    if (!gdk_window_ensure_native(window))
    {
        g_error("Could not create native window needed for GstVideoOverlay!");
        std::cerr << "Could not create native window needed for GstVideoOverlay!";
    }
    std::cout << "Tested if gdk window is native\n";

    /* Retrieve window handler from GDK */
#if defined (GDK_WINDOWING_WIN32)
    window_handle = (guintptr)GDK_WINDOW_HWND(window);
#elif defined (GDK_WINDOWING_QUARTZ)
    window_handle = gdk_quartz_window_get_nsview(window);
#elif defined (GDK_WINDOWING_X11)
    window_handle = GDK_WINDOW_XID(window);
    std::cout << "window_handle " << window_handle << " comes from GDK_WINDOW_XID\n";
#endif

    /* Pass it to playbin, which implements VideoOverlay and will forward it to the video sink */
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(pipeline), window_handle);
}

/* This function is called everytime the video window needs to be redrawn (due to damage/exposure,
 * rescaling, etc). GStreamer takes care of this in the PAUSED and PLAYING states, otherwise,
 * we simply draw a black rectangle to avoid garbage showing up. */
static gboolean draw_cb(GtkWidget *widget, cairo_t *cr, void *data)
{
    GstState state = *(static_cast<GstState *>(data));
    if (state < GST_STATE_PAUSED)
    {
        GtkAllocation allocation;

        /* Cairo is a 2D graphics library which we use here to clean the video window.
        * It is used by GStreamer for other reasons, so it will always be available to us. */
        gtk_widget_get_allocation(widget, &allocation);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
        cairo_fill(cr);
    }

    return FALSE;
}

VideoPlayerView::VideoPlayerView()
    :   m_playerInfo(nullptr),
        m_playbin(nullptr)
{
    memset(&m_UIComponents, 0, sizeof(m_UIComponents));
}

void VideoPlayerView::CreateUI() // TODO: CONNECT ALL THE CALLBACKS
{
    m_UIComponents.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    m_UIComponents.video_window = gtk_drawing_area_new();
    g_signal_connect(m_UIComponents.video_window, "realize", G_CALLBACK(realize_cb), this->m_playbin);
    g_signal_connect(m_UIComponents.video_window, "draw", G_CALLBACK(draw_cb), &(this->m_playerInfo->state));    // TODO: CHECK THIS DATA POINTER, IT USES ONLY THE STATE OF PLAYBACK 
                                                                                                            // IS THE STATE INIT YET? WILL KEEP IT INSIDE APP DATA OR PLAYERINFO?

    m_UIComponents.play_button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_SMALL_TOOLBAR);

    m_UIComponents.pause_button = gtk_button_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_SMALL_TOOLBAR);

    m_UIComponents.stop_button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_SMALL_TOOLBAR);

    m_UIComponents.slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0, 100 , 1);
    gtk_scale_set_draw_value(GTK_SCALE(m_UIComponents.slider), 0);
    // TODO: CONNECT SLIDER CALLBACK

    m_UIComponents.streams_list = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(m_UIComponents.streams_list), FALSE);

    m_UIComponents.controls = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.controls), m_UIComponents.play_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.controls), m_UIComponents.pause_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.controls), m_UIComponents.stop_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.controls), m_UIComponents.slider, TRUE, TRUE, 2);

    m_UIComponents.main_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.main_hbox), m_UIComponents.video_window, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.main_hbox), m_UIComponents.streams_list, FALSE, FALSE, 2);

    m_UIComponents.main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.main_box), m_UIComponents.main_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(m_UIComponents.main_box), m_UIComponents.controls, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(m_UIComponents.main_window), m_UIComponents.main_box);
    gtk_window_set_default_size(GTK_WINDOW(m_UIComponents.main_window), 640, 480);

    gtk_widget_show_all(m_UIComponents.main_window);
}

void VideoPlayerView::RefreshUI()
{
    /* We do not want to update anything unless we are in the PAUSED or PLAYING states */
    if (m_playerInfo->state < GST_STATE_PAUSED)
    {
        return;
    }

    // Considering that the refresh of data was trigged by a time cb in controller and the updated model triggered updates in the view before the refreshUI function was called
    gtk_range_set_range(GTK_RANGE(m_UIComponents.slider), 0, (gdouble)m_playerInfo->duration / GST_SECOND);

    /* Block the "value-changed" signal, so the slider_cb function is not called
    * (which would trigger a seek the user has not requested) */
    g_signal_handler_block(m_UIComponents.slider, m_UIComponents.slider_update_signal_id);
    /* Set the position of the slider to the current pipeline position, in SECONDS */
    gtk_range_set_value(GTK_RANGE(m_UIComponents.slider), (gdouble)m_playerInfo->current_time / GST_SECOND);
    /* Re-enable the signal */
    g_signal_handler_unblock(m_UIComponents.slider, m_UIComponents.slider_update_signal_id);
}

void VideoPlayerView::SetDeleteCallback(delete_callback_fn delete_cb, void *data)
{
    if (m_UIComponents.main_window == nullptr)
    {
        return;
    }
    g_signal_connect(G_OBJECT(m_UIComponents.main_window), "delete-event", G_CALLBACK(delete_cb), data);
}

void VideoPlayerView::SetPlayCallback(btn_callback_fn play_cb, void *data)
{
    if (m_UIComponents.play_button == nullptr)
    {
        return;
    }
    g_signal_connect(G_OBJECT(m_UIComponents.play_button), "clicked", G_CALLBACK(play_cb), data);
}

void VideoPlayerView::SetPauseCallback(btn_callback_fn pause_cb, void *data)
{
    if (m_UIComponents.pause_button == nullptr)
    {
        return;
    }
    g_signal_connect(G_OBJECT(m_UIComponents.pause_button), "clicked", G_CALLBACK(pause_cb), data);
}

void VideoPlayerView::SetStopCallback(btn_callback_fn stop_cb, void *data)
{
    if (m_UIComponents.stop_button == nullptr)
    {
        return;
    }
    g_signal_connect(G_OBJECT(m_UIComponents.stop_button), "clicked", G_CALLBACK(stop_cb), data);
}

void VideoPlayerView::Update(void *data)
{
    m_playerInfo = static_cast<PlayerInfo *>(data);
    PrintPlayerInfo(*m_playerInfo);
}

void VideoPlayerView::SetPipelineObject(GstElement *pipeline)
{
    if (pipeline != nullptr)
    {
        m_playbin = pipeline;
    }
}

static void PrintPlayerInfo(PlayerInfo &player_info)
{
    std::cout   << "Player Info : \n" 
                << "Video URL: " << player_info.video_url 
                << "\nPlayback State: " 
                << GET_STATE_NAME(player_info.state);
    g_print("\nCurrent time: %" GST_TIME_FORMAT "\nDuration: %" GST_TIME_FORMAT, GST_TIME_ARGS(player_info.current_time), GST_TIME_ARGS(player_info.duration));
}