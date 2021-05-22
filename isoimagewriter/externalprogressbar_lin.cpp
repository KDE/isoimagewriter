/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Linux implementation of ExternalProgressBar
// Not implemented yet, contains only stubs


#include <QWidget>

#include "externalprogressbar.h"

// Class with platform-specific data
class ExternalProgressBarPrivate
{
public:
    ExternalProgressBarPrivate();
    ~ExternalProgressBarPrivate();
};

ExternalProgressBarPrivate::ExternalProgressBarPrivate()
{
}

ExternalProgressBarPrivate::~ExternalProgressBarPrivate()
{
}


ExternalProgressBar::ExternalProgressBar(QWidget* mainWindow) :
    d_ptr(new ExternalProgressBarPrivate()),
    m_MaxValue(0)
{
    Q_UNUSED(mainWindow);
}

ExternalProgressBar::~ExternalProgressBar()
{
    DestroyProgressBar();
    delete d_ptr;
}

// Initializes the external progress bar and sets its limits
bool ExternalProgressBar::InitProgressBar(quint64 maxSteps)
{
    m_MaxValue = maxSteps;
    Q_UNUSED(maxSteps);
    return false;
}

// Deinitializes the external progress bar and returns into the normal state
bool ExternalProgressBar::DestroyProgressBar()
{
    return false;
}

// Updates the current progress bar position
bool ExternalProgressBar::SetProgressValue(quint64 currentSteps)
{
    Q_UNUSED(currentSteps);
    return false;
}

// Sets the progress bar to indicate pause
bool ExternalProgressBar::ProgressSetPause()
{
    return false;
}

// Sets the progress bar to indicate an error
bool ExternalProgressBar::ProgressSetError()
{
    return false;
}

