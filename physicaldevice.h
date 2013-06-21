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
