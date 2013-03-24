#ifndef EXTERNALPROGRESSBAR_H
#define EXTERNALPROGRESSBAR_H

////////////////////////////////////////////////////////////////////////////////
// Class for exporting progressbar state to some external component
// At the moment implemented only for the Windows taskbar


#include "common.h"

class ExternalProgressBar
{
public:
    ExternalProgressBar(QWidget* mainWindow);
    ~ExternalProgressBar();

    // Initializes the external progress bar and sets its limits
    bool InitProgressBar(quint64 maxSteps);

    // Deinitializes the external progress bar and returns into the normal state
    bool DestroyProgressBar();

    // Updates the current progress bar position
    bool SetProgressValue(quint64 currentSteps);

    // Sets the progress bar to indicate pause
    bool ProgressSetPause();

    // Sets the progress bar to indicate an error
    bool ProgressSetError();

protected:
    // Maximum counter value for the progress bar
    quint64 m_MaxValue;

#if defined(Q_OS_WIN32)
    // Windows7 Taskbar interface for mirroring the progress bar
    ITaskbarList3* m_Win7TaskbarList;

    // Main window handle for selecting the correct taskbar button
    HWND m_hWnd;
#endif
};

#endif // EXTERNALPROGRESSBAR_H
