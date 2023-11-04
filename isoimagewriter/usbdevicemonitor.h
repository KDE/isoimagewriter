/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef USBDEVICEMONITOR_H
#define USBDEVICEMONITOR_H

////////////////////////////////////////////////////////////////////////////////
// Class implementing monitoring for inserting/removing USB devices


#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QSocketNotifier>

#include "common.h"

class UsbDeviceMonitorPrivate;
class UsbDeviceMonitor : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(UsbDeviceMonitor)

protected:
    UsbDeviceMonitorPrivate* const d_ptr;

public:
    explicit UsbDeviceMonitor(QObject *parent = 0);
    ~UsbDeviceMonitor();
    
    // Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
    bool nativeEventFilter(const QByteArray &theEventType, void *theMessage, qintptr *theResult) override;

protected:
    // Closes handles and frees resources
    void cleanup();

signals:
    // Emitted when device change notification arrives
    void deviceChanged();

public slots:
    // Initializes monitoring for USB devices
    bool startMonitoring();
};

#endif // USBDEVICEMONITOR_H
