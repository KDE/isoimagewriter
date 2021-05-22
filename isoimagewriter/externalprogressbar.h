/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef EXTERNALPROGRESSBAR_H
#define EXTERNALPROGRESSBAR_H

////////////////////////////////////////////////////////////////////////////////
// Class for exporting progressbar state to some external component
// At the moment implemented only for the Windows taskbar


#include "common.h"

class ExternalProgressBarPrivate;
class ExternalProgressBar
{
protected:
    ExternalProgressBarPrivate* const d_ptr;

public:
    explicit ExternalProgressBar(QWidget* mainWindow);
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
};

#endif // EXTERNALPROGRESSBAR_H
