/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Linux implementation of UsbDeviceMonitor

#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/StorageDrive>

#include "usbdevicemonitor.h"

UsbDeviceMonitor::UsbDeviceMonitor(QObject *parent)
    : QObject(parent)
    , d_ptr(nullptr)
{
}

UsbDeviceMonitor::~UsbDeviceMonitor() = default;

void UsbDeviceMonitor::cleanup()
{
}

bool UsbDeviceMonitor::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
    return false;
}

bool UsbDeviceMonitor::startMonitoring()
{
    auto notifier = Solid::DeviceNotifier::instance();
    auto refresh = [this](const QString &udi) {
        Solid::Device device(udi);
        if (!device.is<Solid::StorageDrive>()) {
            Q_EMIT deviceChanged();
        }
    };
    connect(notifier, &Solid::DeviceNotifier::deviceAdded, this, refresh);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved, this, refresh);
    return true;
}
