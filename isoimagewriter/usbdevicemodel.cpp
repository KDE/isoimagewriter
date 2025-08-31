#/*
#  * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
#  * SPDX-License-Identifier: GPL-3.0-or-later
#  */

#include "usbdevicemodel.h"
#include "platform.h"
#include <QDebug>

UsbDeviceModel::UsbDeviceModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_monitor = new UsbDeviceMonitor(this);
    // When the monitor detects a change, we re-scan for devices
    connect(m_monitor, &UsbDeviceMonitor::deviceChanged, this, &UsbDeviceModel::onDeviceChanged);
    m_monitor->startMonitoring();

    // Perform an initial scan
    enumFlashDevices();
}

UsbDeviceModel::~UsbDeviceModel()
{
    cleanUp();
}

int UsbDeviceModel::rowCount(const QModelIndex &parent) const
{
    // The number of items in the list is the number of devices we have.
    if (parent.isValid())
        return 0;
    return m_devices.count();
}

QVariant UsbDeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_devices.count())
        return QVariant();

    UsbDevice *device = m_devices.at(index.row());

    switch (role) {
    case DisplayNameRole:
        return device->displayName();
    case DevicePointerRole:
        return QVariant::fromValue(device);
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> UsbDeviceModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DisplayNameRole] = "displayName";
    roles[DevicePointerRole] = "device";
    return roles;
}

UsbDevice *UsbDeviceModel::getDevice(int index) const
{
    if (index < 0 || index >= m_devices.count()) {
        return nullptr;
    }
    return m_devices.at(index);
}

void UsbDeviceModel::onDeviceChanged()
{
    qDebug() << "Device change detected, rescanning...";
    enumFlashDevices();
}

void UsbDeviceModel::enumFlashDevices()
{
    beginResetModel(); // Notifies views that the model is about to change drastically
    cleanUp(); // Clear the old list

    platformEnumFlashDevices(addFlashDeviceCallback, this);

    endResetModel(); // Notifies views that the model has changed

    emit countChanged();
    emit hasDevicesChanged();

    qDebug() << "Enumeration complete. Found" << count() << "devices.";
}

void UsbDeviceModel::cleanUp()
{
    qDeleteAll(m_devices);
    m_devices.clear();
}

void UsbDeviceModel::addFlashDeviceCallback(void *cbParam, UsbDevice *device)
{
    UsbDeviceModel *model = static_cast<UsbDeviceModel *>(cbParam);
    model->m_devices.append(device);
}
