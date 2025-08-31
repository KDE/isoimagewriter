/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
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
    explicit PhysicalDevice(const QString &name);

    // Opens the selected device in WriteOnly mode
    virtual bool open();

protected:
#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    int getDescriptor();
#endif
#if defined(Q_OS_WIN32)
    HANDLE m_fileHandle;
#endif
};

#endif // PHYSICALDEVICE_H
