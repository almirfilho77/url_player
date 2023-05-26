#ifndef __COMMON_H__
#define __COMMON_H__

#include <functional>
#include <string>

#include <gtk/gtk.h>
#include <gst/gst.h>

#include "Logger.h"

/* Structure to contain all our information, so we can pass it around */
typedef struct _ApplicationData
{
    GstElement *playbin;           /* Our one and only pipeline */
    const char *video_url;          /* url to play */

    GtkWidget *slider;              /* Slider widget to keep track of current position */
    GtkWidget *streams_list;        /* Text widget to display info about the streams */
    gulong slider_update_signal_id; /* Signal ID for the slider update signal */

    GstState state;                 /* Current state of the pipeline */
    gint64 duration;                /* Duration of the clip, in nanoseconds */
} ApplicationData;

typedef struct _PlayerInfo
{
    std::string video_url;
    GstState state;
    gint64 current_time;
    gint64 duration;
} PlayerInfo;

typedef struct _PlaybackInfo
{
    unsigned int max_value;
    unsigned int min_value;
    unsigned int step;
} PlaybackInfo;

typedef void (*btn_callback_fn)(GtkButton *, void *);
typedef void (*delete_callback_fn)(GtkWidget *, GdkEvent *, void *);

#define GET_STATE_NAME(x) ( (x == GST_STATE_VOID_PENDING) ? "VOID PENDING" : \
                            (x == GST_STATE_NULL) ? "NULL" : \
                            (x == GST_STATE_READY) ? "READY" : \
                            (x == GST_STATE_PAUSED) ? "PAUSED" : \
                            (x == GST_STATE_PLAYING) ? "PLAYING" : "UNKNOWN")

#endif /* __COMMON_H__ */