/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>, 2023 Jonathan Esk-Riddell <jr@jriddell.org>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// This file contains Linux implementation of platform-dependent functions

#include "common.h"

#include <KLocalizedString>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QtDBus/QtDBus>
#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>

#include "mainapplication.h"
#include "usbdevice.h"

typedef QHash<QString, QVariantMap> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

UsbDevice* handleObject(const QDBusObjectPath &object_path, const InterfacesAndProperties &interfaces_and_properties) {
    QRegularExpression numberRE("[0-9]$");
    QRegularExpressionMatch numberREMatch = numberRE.match(object_path.path());
    QRegularExpression mmcRE("[0-9]p[0-9]$");
    QRegularExpressionMatch mmcREMatch = mmcRE.match(object_path.path());
    QDBusObjectPath driveId = qvariant_cast<QDBusObjectPath>(interfaces_and_properties["org.freedesktop.UDisks2.Block"]["Drive"]);

    QDBusInterface driveInterface("org.freedesktop.UDisks2", driveId.path(), "org.freedesktop.UDisks2.Drive", QDBusConnection::systemBus());
    UsbDevice* deviceData = new UsbDevice;

    if ((numberREMatch.hasMatch() && !object_path.path().startsWith("/org/freedesktop/UDisks2/block_devices/mmcblk")) ||
            mmcREMatch.hasMatch())
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
        //bool isoLayout = interfaces_and_properties["org.freedesktop.UDisks2.Block"]["IdType"].toString() == "iso9660";

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

        //qDebug() << "New drive" << driveId.path() << "-" << name << "(" << size << "bytes;" << (isValid ? "removable;" : "nonremovable;") << connectionBus << ")";

        deviceData->m_PhysicalDevice = object_path.path();
        deviceData->m_Volumes = QStringList{ deviceData->m_PhysicalDevice };
        deviceData->m_SectorSize = 512;
        deviceData->m_Size = size;
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
    QDBusInterface* m_objManager = new QDBusInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
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
        UsbDevice* deviceData = nullptr;
        deviceData = handleObject(i, introspection[i]);
        if (deviceData) {
            callback(cbParam, deviceData);
        }
    }

    return true;
}

bool ensureElevated()
{
    // on Linux we use udisks2 which uses polkit to run necessary bits as root
    return true;
}
