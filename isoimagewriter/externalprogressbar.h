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
