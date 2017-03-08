/*
 * Copyright 2016 ROSA
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

