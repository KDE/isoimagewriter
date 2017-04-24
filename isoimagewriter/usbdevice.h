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

#ifndef USBDEVICE_H
#define USBDEVICE_H

////////////////////////////////////////////////////////////////////////////////
// Class for storing information about USB flash disk

#include "common.h"

#include <QStringList>

#include <KLocalizedString>

class UsbDevice
{
public:
    UsbDevice() :
        m_VisibleName(i18n("Unknown Device")),
        m_Volumes(),
        m_Size(0),
        m_SectorSize(512),
        m_PhysicalDevice("") {}

    // Formats the device description for GUI
    // The format is: "<volume(s)> - <user-friendly name> (<size in megabytes>)"
    QString formatDisplayName() const {
        return ((m_Volumes.size() == 0) ? i18n("<unmounted>") : m_Volumes.join(", ")) + " - " + m_VisibleName + " (" + QString::number(alignNumberDiv(m_Size, DEFAULT_UNIT)) + " " + i18n("MiB") + ")";
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
