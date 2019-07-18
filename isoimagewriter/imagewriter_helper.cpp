/*
 * Copyright 2017 Jonathan Riddell <jr@jriddell.org>
 * 
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

#include "imagewriter_helper.h"
#include "isoimagewriter_debug.h"
#include "imagewriter.h"
#include "usbdevice.h"

#include <KLocalizedString>
#include <KAuthActionReply>

#include <QProcess>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QThread>

#include <stdio.h>
#include <iostream>
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

    UsbDevice* selectedDevice = new UsbDevice();
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

    ImageWriter* writer = new ImageWriter(zeroing ? "" : imageFile, selectedDevice);
    writer->writeImage();

    return ActionReply::SuccessReply();
}

KAUTH_HELPER_MAIN("org.kde.isoimagewriter", ImageWriterHelper)
