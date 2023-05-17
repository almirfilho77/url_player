#include "Application.h"
#include "Common.h"

int main(int argc, char *argv[])
{
    /* Init GTK */
    gtk_init(&argc, &argv);
    
    /* Init GStreamer */
    gst_init(&argc, &argv);

    // Create the application
    Application video_player_app("https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm");

    /* Start the GTK main loop. We will not regain control until gtk_main_quit is called. */
    gtk_main ();

    return 0;
}