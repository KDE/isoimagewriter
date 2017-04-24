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

#ifndef PHYSICALDEVICE_H
#define PHYSICALDEVICE_H

////////////////////////////////////////////////////////////////////////////////
// Class implementing write-only physical device


#include <QFile>

#include "common.h"

class PhysicalDevice : public QFile
{
    Q_OBJECT
public:
    PhysicalDevice(const QString& name);

    // Opens the selected device in WriteOnly mode
    virtual bool open();

protected:
#if defined(Q_OS_WIN32)
    HANDLE m_fileHandle;
#endif
};

#endif // PHYSICALDEVICE_H
