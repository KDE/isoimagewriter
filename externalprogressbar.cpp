////////////////////////////////////////////////////////////////////////////////
// Implementation of ExternalProgressBar


#include <QWidget>

#include "externalprogressbar.h"

ExternalProgressBar::ExternalProgressBar(QWidget* mainWindow) :
    m_MaxValue(0)
{
#if defined(Q_OS_WIN32)
    // Get the taskbar object (if NULL is returned it won't be used - e.g. on pre-7 Windows versions)
    CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, reinterpret_cast<void**>(&m_Win7TaskbarList));
    // Store the window handle. In Windows winId() returns HWND.
    m_hWnd = reinterpret_cast<HWND>(mainWindow->winId());
#else
    Q_UNUSED(mainWindow);
#endif
}

ExternalProgressBar::~ExternalProgressBar()
{
    DestroyProgressBar();
#if defined(Q_OS_WIN32)
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->Release();
#endif
}

// Initializes the external progress bar and sets its limits
bool ExternalProgressBar::InitProgressBar(quint64 maxSteps)
{
    bool res = true;
    m_MaxValue = maxSteps;
#if defined(Q_OS_WIN32)
    // When we set the progress value, TBPF_NORMAL is set automatically
    if (m_Win7TaskbarList != NULL)
        res = (m_Win7TaskbarList->SetProgressValue(m_hWnd, 0, maxSteps) == S_OK);
#else
    Q_UNUSED(maxSteps);
#endif
    return res;
}

// Deinitializes the external progress bar and returns into the normal state
bool ExternalProgressBar::DestroyProgressBar()
{
    bool res = true;
#if defined(Q_OS_WIN32)
    if (m_Win7TaskbarList != NULL)
        res = (m_Win7TaskbarList->SetProgressState(m_hWnd, TBPF_NOPROGRESS) == S_OK);
#endif
    return res;
}

// Updates the current progress bar position
bool ExternalProgressBar::SetProgressValue(quint64 currentSteps)
{
    bool res = true;
#if defined(Q_OS_WIN32)
    if (m_Win7TaskbarList != NULL)
        res = (m_Win7TaskbarList->SetProgressValue(m_hWnd, currentSteps, m_MaxValue) == S_OK);
#else
    Q_UNUSED(currentSteps);
#endif
    return res;
}

// Sets the progress bar to indicate pause
bool ExternalProgressBar::ProgressSetPause()
{
    bool res = true;
#if defined(Q_OS_WIN32)
    if (m_Win7TaskbarList != NULL)
        res = (m_Win7TaskbarList->SetProgressState(m_hWnd, TBPF_PAUSED) == S_OK);
#endif
    return res;
}

// Sets the progress bar to indicate an error
bool ExternalProgressBar::ProgressSetError()
{
    bool res = true;
#if defined(Q_OS_WIN32)
    if (m_Win7TaskbarList != NULL)
        res = (m_Win7TaskbarList->SetProgressState(m_hWnd, TBPF_ERROR) == S_OK);
#endif
    return res;
}

