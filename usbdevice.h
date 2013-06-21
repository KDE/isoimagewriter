#ifndef USBDEVICE_H
#define USBDEVICE_H

////////////////////////////////////////////////////////////////////////////////
// Class for storing information about USB flash disk

#include "common.h"

#include <QStringList>

class UsbDevice
{
public:
    UsbDevice() :
        m_VisibleName(QObject::tr("Unknown Device")),
        m_Volumes(),
        m_Size(0),
        m_SectorSize(512),
        m_PhysicalDevice("") {}

    // Formats the device description for GUI
    // The format is: "<volume(s)> - <user-friendly name> (<size in megabytes>)"
    QString formatDisplayName() const {
        return ((m_Volumes.size() == 0) ? QObject::tr("<unmounted>") : m_Volumes.join(", ")) + " - " + m_VisibleName + " (" + QString::number(alignNumberDiv(m_Size, DEFAULT_UNIT)) + " " + QObject::tr("MB") + ")";
    }

    // User-friendly name of the device
    QString     m_VisibleName;
    // List of mounted volumes from this device
    QStringList m_Volumes;
    // Size of the device
    quint64     m_Size;
    // Sector size
    quint32     m_SectorSize;
    // System name of the physical disk
    QString     m_PhysicalDevice;
};

Q_DECLARE_METATYPE(UsbDevice*)


#endif // USBDEVICE_H
