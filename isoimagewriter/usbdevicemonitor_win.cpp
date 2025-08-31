/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Windows implementation of UsbDeviceMonitor

#include "usbdevicemonitor.h"
#include "usbdevicemonitor_win_p.h"

#include <QApplication>

// Private class implementation

UsbDeviceMonitorPrivate::UsbDeviceMonitorPrivate()
{
}

UsbDeviceMonitorPrivate::~UsbDeviceMonitorPrivate()
{
}

// Main class implementation

UsbDeviceMonitor::UsbDeviceMonitor(QObject *parent)
    : QObject(parent)
    , d_ptr(new UsbDeviceMonitorPrivate())
{
}

UsbDeviceMonitor::~UsbDeviceMonitor()
{
    cleanup();
    delete d_ptr;
}

// Closes handles and frees resources
void UsbDeviceMonitor::cleanup()
{
}

// Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
bool UsbDeviceMonitor::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
#else
bool UsbDeviceMonitor::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
#endif
{
    Q_UNUSED(eventType);

    MSG *msg = static_cast<MSG *>(message);
    if ((msg->message == WM_DEVICECHANGE) && ((msg->wParam == DBT_DEVICEARRIVAL) || (msg->wParam == DBT_DEVICEREMOVECOMPLETE))) {
        // If the event was caused by adding or removing a device, mark the WinAPI message as processed
        // and emit the notification signal
        *result = TRUE;
        emit deviceChanged();
        return true;
    }
    return false;
}

bool UsbDeviceMonitor::startMonitoring()
{
    // In Windows we use QAbstractNativeEventFilter interface implementation and process native Windows messages
    qApp->installNativeEventFilter(this);
    return true;
}
