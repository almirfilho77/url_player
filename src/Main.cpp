#include "Application.h"
#include "Common.h"

int main(int argc, char *argv[])
{
    Logger::Init();
    _DEBUG("Begin of main function!");

    /* Init GTK */
    gtk_init(&argc, &argv);
    _DEBUG("GTK init done!");
    
    /* Init GStreamer */
    gst_init(&argc, &argv);
    _DEBUG("Gstreamer init done!");

    // Create the application
    Application video_player_app("https://gstreamer.freedesktop.org/data/media/sintel_trailer-480p.webm");
    _DEBUG("Create Application done!");

    /* Start the GTK main loop. We will not regain control until gtk_main_quit is called. */
    _DEBUG("About to enter GTK main!");
    gtk_main ();
    _DEBUG("GTK main quit!");

    return 0;
}