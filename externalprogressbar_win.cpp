////////////////////////////////////////////////////////////////////////////////
// Windows implementation of ExternalProgressBar


#include <QWidget>

#include "externalprogressbar.h"

// Class with platform-specific data
class ExternalProgressBarPrivate
{
public:
    ExternalProgressBarPrivate();
    ~ExternalProgressBarPrivate();

    // Windows7 Taskbar interface for mirroring the progress bar
    ITaskbarList3* m_Win7TaskbarList;

    // Main window handle for selecting the correct taskbar button
    HWND m_hWnd;
};

ExternalProgressBarPrivate::ExternalProgressBarPrivate() :
    m_Win7TaskbarList(NULL),
    m_hWnd(NULL)
{
}

ExternalProgressBarPrivate::~ExternalProgressBarPrivate()
{
}


ExternalProgressBar::ExternalProgressBar(QWidget* mainWindow) :
    d_ptr(new ExternalProgressBarPrivate()),
    m_MaxValue(0)
{
    // Get the taskbar object (if NULL is returned it won't be used - e.g. on pre-7 Windows versions)
    CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, reinterpret_cast<void**>(&d_ptr->m_Win7TaskbarList));
    // Store the window handle. In Windows winId() returns HWND.
    d_ptr->m_hWnd = reinterpret_cast<HWND>(mainWindow->winId());
}

ExternalProgressBar::~ExternalProgressBar()
{
    DestroyProgressBar();
    if (d_ptr->m_Win7TaskbarList != NULL)
        d_ptr->m_Win7TaskbarList->Release();
    delete d_ptr;
}

// Initializes the external progress bar and sets its limits
bool ExternalProgressBar::InitProgressBar(quint64 maxSteps)
{
    bool res = true;
    m_MaxValue = maxSteps;

    // When we set the progress value, TBPF_NORMAL is set automatically
    if (d_ptr->m_Win7TaskbarList != NULL)
        res = (d_ptr->m_Win7TaskbarList->SetProgressValue(d_ptr->m_hWnd, 0, maxSteps) == S_OK);

    return res;
}

// Deinitializes the external progress bar and returns into the normal state
bool ExternalProgressBar::DestroyProgressBar()
{
    bool res = true;

    if (d_ptr->m_Win7TaskbarList != NULL)
        res = (d_ptr->m_Win7TaskbarList->SetProgressState(d_ptr->m_hWnd, TBPF_NOPROGRESS) == S_OK);

    return res;
}

// Updates the current progress bar position
bool ExternalProgressBar::SetProgressValue(quint64 currentSteps)
{
    bool res = true;

    if (d_ptr->m_Win7TaskbarList != NULL)
        res = (d_ptr->m_Win7TaskbarList->SetProgressValue(d_ptr->m_hWnd, currentSteps, m_MaxValue) == S_OK);

    return res;
}

// Sets the progress bar to indicate pause
bool ExternalProgressBar::ProgressSetPause()
{
    bool res = true;

    if (d_ptr->m_Win7TaskbarList != NULL)
        res = (d_ptr->m_Win7TaskbarList->SetProgressState(d_ptr->m_hWnd, TBPF_PAUSED) == S_OK);

    return res;
}

// Sets the progress bar to indicate an error
bool ExternalProgressBar::ProgressSetError()
{
    bool res = true;

    if (d_ptr->m_Win7TaskbarList != NULL)
        res = (d_ptr->m_Win7TaskbarList->SetProgressState(d_ptr->m_hWnd, TBPF_ERROR) == S_OK);

    return res;
}

