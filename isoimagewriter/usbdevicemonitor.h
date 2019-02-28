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

protected:
    UsbDeviceMonitorPrivate* const d_ptr;

public:
    explicit UsbDeviceMonitor(QObject *parent = 0);
    ~UsbDeviceMonitor();
    
    // Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages (Windows)
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

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
