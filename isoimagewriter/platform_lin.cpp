/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// This file contains Linux implementation of platform-dependent functions

#include <KLocalizedString>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QtDBus/QtDBus>
#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/StorageDrive>

#include "mainapplication.h"
#include "usbdevice.h"

typedef QHash<QString, QVariantMap> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

UsbDevice* handleObject(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegExp numberRE("[0-9]$");
    QRegExp mmcRE("[0-9]p[0-9]$");
    QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);

    QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
    UsbDevice* deviceData = new UsbDevice;

    if ((numberRE.indexIn(object_path.path()) >= 0 && !object_path.path().startsWith("/org/freedesktop/UDisks2/block_devices/mmcblk")) ||
            mmcRE.indexIn(object_path.path()) >= 0)
        return nullptr;

    if (!driveId.path().isEmpty() && driveId.path() != "/") {
        bool portable = driveInterface.property("Removable").toBool();
        bool optical = driveInterface.property("Optical").toBool();
        bool containsMedia = driveInterface.property("MediaAvailable").toBool();
        QString connectionBus = driveInterface.property("ConnectionBus").toString().toLower();
        bool isValid = containsMedia && !optical && (portable || connectionBus == "usb");

        QString vendor = driveInterface.property("Vendor").toString();
        QString model = driveInterface.property("Model").toString();
        uint64_t size = driveInterface.property("Size").toULongLong();
        bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

        QString name;
        if (vendor.isEmpty())
            if (model.isEmpty())
                name = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Device"].toByteArray();
            else
                name = model;
        else
            if (model.isEmpty())
                name = vendor;
            else
                name = QString("%1 %2").arg(vendor).arg(model);

        qDebug() << "New drive" << driveId.path() << "-" << name << "(" << size << "bytes;" << (isValid ? "removable;" : "nonremovable;") << connectionBus << ")";

        deviceData->m_PhysicalDevice = object_path.path();
        deviceData->m_Volumes = QStringList{ deviceData->m_PhysicalDevice };
        /*
        const QString logicalBlockSizeFile = QStringLiteral("/sys/dev/block/%1:%2/queue/logical_block_size").arg(block->deviceMajor())
                                                                                                            .arg(block->deviceMinor());
        deviceData->m_SectorSize = readFileContents(logicalBlockSizeFile).toUInt();
        if (deviceData->m_SectorSize == 0)
            deviceData->m_SectorSize = 512;
        */
        deviceData->m_SectorSize = 512;
        deviceData->m_Size = driveInterface.property("Size").toULongLong();
        deviceData->m_VisibleName = name;

        if (isValid) {
            return deviceData;
        }
    }
    return nullptr;
}

bool platformEnumFlashDevices(AddFlashDeviceCallbackProc callback, void* cbParam)
{

    qDBusRegisterMetaType<InterfacesAndProperties>();
    qDBusRegisterMetaType<DBusIntrospection>();
    qDebug() << "XXX platformEnumFlashDevices()";
    QDBusInterface* m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    //QDBusReply<int> reply = m_objManager->call("GetManagedObjects");
    QDBusPendingReply<DBusIntrospection> reply = m_objManager->asyncCall("GetManagedObjects");
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "Could not read drives from UDisks:" << reply.error().name() << reply.error().message();
        return false;
    }
    DBusIntrospection introspection = reply.argumentAt<0>();
    for (auto i : introspection.keys()) {
        // ignore anything not block device
        if (!i.path().startsWith("/org/freedesktop/UDisks2/block_devices")) {
            continue;
        }
        qDebug() << "XXX platformEnumFlashDevices() path: " << i.path();

        UsbDevice* deviceData = nullptr;
        deviceData = handleObject(i, introspection[i]);
        if (deviceData) {
            callback(cbParam, deviceData);
        }
    }

    /* Old Code
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : devices) {
        if (!device.is<Solid::StorageDrive>()) {
            qDebug() << "Ignoring" << device.displayName() << device.udi();
            continue;
        }

        auto storageDrive = device.as<Solid::StorageDrive>();
        auto block = device.as<Solid::Block>();
        if (storageDrive->isInUse()) {
            qDebug() << "Skipping" << device.displayName() << device.udi();
            continue;
        }

        UsbDevice* deviceData = new UsbDevice;
        deviceData->m_PhysicalDevice = block->device();
        deviceData->m_Volumes = QStringList{ deviceData->m_PhysicalDevice };

        const QString logicalBlockSizeFile = QStringLiteral("/sys/dev/block/%1:%2/queue/logical_block_size").arg(block->deviceMajor())
                                                                                                            .arg(block->deviceMinor());
        deviceData->m_SectorSize = readFileContents(logicalBlockSizeFile).toUInt();
        if (deviceData->m_SectorSize == 0)
            deviceData->m_SectorSize = 512;
        deviceData->m_Size = storageDrive->size();
        deviceData->m_VisibleName = device.displayName();

        callback(cbParam, deviceData);
    }
    */
    return true;
}

bool ensureElevated()
{
    // on Linux we use KAuth which uses polkit to run necessary bits as root
    return true;
}
