/*
    SPDX-FileCopyrightText: 2016 ROSA, 2016 Martin Bříza <mbriza@redhat.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Implementation of PhysicalDevice

#include <KLocalizedString>
#include <QtDBus>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <fcntl.h>

#include "physicaldevice.h"

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

PhysicalDevice::PhysicalDevice(const QString& name) :
    QFile(name)
{
}

// Opens the selected device in WriteOnly mode
bool PhysicalDevice::open()
{
#if defined(Q_OS_WIN32)
    DWORD bret;

    // In Windows QFile with write mode uses disposition OPEN_ALWAYS, but WinAPI
    // requires OPEN_EXISTING for physical devices. Therefore we have to use native API.
    m_fileHandle = CreateFile(
        reinterpret_cast<const wchar_t*>(fileName().utf16()),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
        NULL
    );
    if (m_fileHandle == INVALID_HANDLE_VALUE)
    {
        setErrorString(errorMessageFromCode());
        return false;
    }
    // Lock the opened device
    if (!DeviceIoControl(m_fileHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
    {
        setErrorString(formatErrorMessageFromCode(i18n("Could not acquire lock:")));
        return false;
    }
    // Construct QFile around the device handle; close() will now close the handle automatically
    if (QFile::open(_open_osfhandle(reinterpret_cast<intptr_t>(m_fileHandle), 0), QIODevice::WriteOnly | QIODevice::Unbuffered, AutoCloseHandle))
        return true;
    else
    {
        CloseHandle(m_fileHandle);
        return false;
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    // Simply use QFile, it works fine in Linux
    // TODO: Use system call open with O_DIRECT
    //return QFile::open(QIODevice::WriteOnly);
    qDebug() << "XXX getDescriptor() " << fileName();
    int fd = getDescriptor();
    qDebug() << "XXX getDescriptor() done";
    return QFile::open(fd, QIODevice::WriteOnly);
#else
    return false;
#endif
}

int PhysicalDevice::getDescriptor() {
    // fileName == e.g. /org/freedesktop/UDisks2/block_devices/sda
    // drivePath == e.g. /org/freedesktop/UDisks2/drives/JetFlash_Transcend_8GB_2H1NKR5V
    qDebug() << "XXX getDescriptor() 1";
    QDBusInterface device("org.freedesktop.UDisks2", fileName(), "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
    QString drivePath = qvariant_cast<QDBusObjectPath>(device.property("Drive")).path();
    QDBusInterface manager("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", "org.freedesktop.DBus.ObjectManager", QDBusConnection::systemBus());
    QDBusMessage message = manager.call("GetManagedObjects");
    qDebug() << "XXX getDescriptor() 2";

    if (message.arguments().length() == 1) {
        QDBusArgument arg = qvariant_cast<QDBusArgument>(message.arguments().first());
        DBusIntrospection objects;
        arg >> objects;
        for (auto i : objects.keys()) {
            if (objects[i].contains("org.freedesktop.UDisks2.Filesystem")) {
                QString currentDrivePath = qvariant_cast<QDBusObjectPath>(objects[i]["org.freedesktop.UDisks2.Block"]["Drive"]).path();
                if (currentDrivePath == drivePath) {
                    QDBusInterface partition("org.freedesktop.UDisks2", i.path(), "org.freedesktop.UDisks2.Filesystem", QDBusConnection::systemBus());
                    message = partition.call("Unmount", Properties { {"force", true} });
                }
            }
        }
    } else {
        setErrorString(message.errorMessage());
        qDebug() << "XXX getDescriptor() error 1";
        return -1;
    }

    QDBusReply<QDBusUnixFileDescriptor> reply = device.call(QDBus::Block, "OpenDevice", "rw", Properties{{"flags", O_DIRECT | O_SYNC | O_CLOEXEC}} );
    QDBusUnixFileDescriptor fd = reply.value();

    if (!fd.isValid()) {
        setErrorString(reply.error().message());
        qDebug() << "XXX getDescriptor() error 2: " << reply.error().message();
        return -1;
    }

    qDebug() << "XXX getDescriptor() return fd: " << fd.fileDescriptor();
    return fd.fileDescriptor();
}
