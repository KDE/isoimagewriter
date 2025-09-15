#/*
#  * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
#  * SPDX-License-Identifier: GPL-3.0-or-later
#  */

#pragma once

#include "usbdevice.h"
#include "usbdevicemonitor.h"
#include <QAbstractListModel>
#include <QObject>
#include <QQmlEngine>
#include <qqmlregistration.h>

class UsbDeviceModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(bool hasDevices READ hasDevices NOTIFY hasDevicesChanged)

public:
    enum UsbDeviceRoles {
        DisplayNameRole = Qt::UserRole + 1,
        DevicePointerRole
    };

    explicit UsbDeviceModel(QObject *parent = nullptr);
    ~UsbDeviceModel();

    // QAbstractListModel interface
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Properties
    int count() const
    {
        return m_devices.size();
    }
    bool hasDevices() const
    {
        return !m_devices.isEmpty();
    }

    // Invokable methods for QML
    Q_INVOKABLE UsbDevice *getDevice(int index) const;

public slots:
    void onDeviceChanged();

signals:
    void countChanged();
    void hasDevicesChanged();

private:
    void enumFlashDevices();
    void cleanUp();
    static void addFlashDeviceCallback(void *cbParam, UsbDevice *device);

    QList<UsbDevice *> m_devices;
    UsbDeviceMonitor *m_monitor;
};
