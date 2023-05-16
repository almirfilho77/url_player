#include "Application.h"

Application::Application(const char *video_url)
{
    /* Init our data structure */
    memset(m_appData, 0, sizeof(*m_appData));
    m_appData->video_url = video_url;

    // Init MVC
    // Init model
    m_videoPlayerModel = new VideoPlayerModel();

    // Init View
    m_videoPlayerView = new VideoPlayerView(m_appData);
    m_videoPlayerModel->Subscribe(m_videoPlayerView);

    // Init Controller
    m_videoPlayerController = new VideoPlayerController(m_appData, m_videoPlayerModel, m_videoPlayerView); // TODO: INIT PIPELINE
    
    // Create UI in the View
    m_videoPlayerView->CreateUI();

    // Set play, pause, stop, etc callbacks to be able to modify the model
    m_videoPlayerController->ConnectUISignals();

    // Update model info (PlayerInfo)
}

Application::~Application()
{
    m_videoPlayerModel->Unsubscribe(m_videoPlayerView);
}