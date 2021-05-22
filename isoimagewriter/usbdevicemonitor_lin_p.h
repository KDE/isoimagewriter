/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
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
    explicit UsbDeviceMonitorPrivate(QObject *parent = nullptr);
    virtual ~UsbDeviceMonitorPrivate();

    UsbDeviceMonitor* q_ptr;

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
