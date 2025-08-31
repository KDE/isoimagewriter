/*
    SPDX-FileCopyrightText: 2017 Jonathan Riddell <jr@jriddell.org>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "imagewriter_helper.h"
#include "imagewriter.h"
#include "isoimagewriter_debug.h"
#include "usbdevice.h"

#include <KLocalizedString>
#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#else
#include <KAuthActionReply>
#endif

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QThread>

#include <iostream>
#include <stdio.h>
#include <time.h>

ImageWriterHelper::ImageWriterHelper()
{
    KLocalizedString::setApplicationDomain("imagewriter");
}

ActionReply ImageWriterHelper::write(const QVariantMap &args)
{
    qDebug() << "ImageWriterHelper::writefile()";
    bool zeroing = args["zeroing"].toBool();
    QString imageFile = args["imagefile"].toString();
    QString visibleName = args[QStringLiteral("usbdevice_visiblename")].toString();
    QStringList volumes;
    volumes << args[QStringLiteral("usbdevice_volumes")].toString();
    quint64 size = args[QStringLiteral("usbdevice_size")].toString().toUInt();
    quint32 sectorSize = args[QStringLiteral("usbdevice_sectorsize")].toUInt();
    QString physicalDevice = args[QStringLiteral("usbdevice_physicaldevice")].toString();

    UsbDevice *selectedDevice = new UsbDevice();
    selectedDevice->m_VisibleName = visibleName;
    selectedDevice->m_Volumes = volumes;
    selectedDevice->m_Size = size;
    selectedDevice->m_SectorSize = sectorSize;
    selectedDevice->m_PhysicalDevice = physicalDevice;

    qDebug() << "ImageWriterHelper::writefile() zeroing:" << zeroing;
    qDebug() << "ImageWriterHelper::writefile() imageFile:" << imageFile;
    qDebug() << "ImageWriterHelper::writefile() physicalDevice:" << physicalDevice;
    qDebug() << "ImageWriterHelper::writefile() volumes:" << volumes[0];
    qDebug() << "ImageWriterHelper::writefile() size:" << size;
    qDebug() << "ImageWriterHelper::writefile() sectorSize:" << sectorSize;

    ImageWriter *writer = new ImageWriter(zeroing ? "" : imageFile, selectedDevice);
    writer->writeImage();

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.isoimagewriter", ImageWriterHelper)

#include "moc_imagewriter_helper.cpp"
