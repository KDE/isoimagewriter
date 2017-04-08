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

#ifndef USBDEVICEMONITOR_LIN_P_H
#define USBDEVICEMONITOR_LIN_P_H

#include <QObject>
#include <QSocketNotifier>

// Class with platform-specific data
class UsbDeviceMonitor;
class UsbDeviceMonitorPrivate : public QObject
{
    Q_OBJECT

public:
    explicit UsbDeviceMonitorPrivate(QObject *parent = 0);
    virtual ~UsbDeviceMonitorPrivate();

    UsbDeviceMonitor* q_ptr;

    // Handle to dynamically loaded udev library
    void* m_udevLib;
    // udev library context
    struct udev* m_udev;
    // udev device monitor handle
    struct udev_monitor* m_udevMonitor;
    // Watcher for udev monitor socket
    QSocketNotifier* m_udevNotifier;

public slots:
    // Processes udev socket notification
    void processUdevNotification(int socket);
};


#endif // USBDEVICEMONITOR_LIN_P_H
