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

ActionReply ImageWriterHelper::writeimage(const QVariantMap &args)
{
    qDebug() << "ImageWriterHelper::writeimage()";
    ActionReply reply;
    QString filename = args["filename"].toString();
    QFile file("/root/" + filename);

    if (!file.open(QIODevice::ReadWrite)) {
       reply = ActionReply::HelperErrorReply();
       reply.setError(file.error());

       return reply;
    }

    QTextStream stream(&file);
    stream << "something" << endl;

    QVariantMap retdata;
    retdata["contents"] = "something";

    reply.setData(retdata);
    qDebug() << "I'm in the helper";
    for (int i = 0; i < 5; i++) {
        if (KAuth::HelperSupport::isStopped()) {
            qDebug() << "HELPER STOPPED";
            reply.setError(KAuth::ActionReply::HelperErrorType);
            reply.setErrorDescription("foo");
            return reply;
        }
        qDebug() << "helper: " << i;
        QFile file("/tmp/foo");

        if (!file.open(QIODevice::ReadWrite)) {
            reply = ActionReply::HelperErrorReply();
            reply.setError(file.error());
            return reply;
        }
        QTextStream stream(&file);
        stream << i << endl;
        file.close();
        struct timespec ts = { 1000 / 1000, (1000 % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
        KAuth::HelperSupport::progressStep(i);
        QVariantMap progress;
        progress["value"] = i;
        KAuth::HelperSupport::progressStep(progress);
    }
    return reply;
}

ActionReply ImageWriterHelper::writefile(const QVariantMap &args)
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
    /*
    ImageWriter* writer = new ImageWriter(zeroing ? "" : imageFile, selectedDevice);
    QThread *writerThread = new QThread(this);

    // Connect start and end signals
    connect(writerThread, &QThread::started, writer, &ImageWriter::writeImage);

    // When writer finishes its job, quit the thread
    connect(writer, &ImageWriter::finished, writerThread, &QThread::quit);

    // Guarantee deleting the objects after completion
    connect(writer, &ImageWriter::finished, writer, &ImageWriter::deleteLater);
    connect(writerThread, &QThread::finished, writerThread, &QThread::deleteLater);
    connect(writerThread, &QThread::finished, this, &ImageWriter::threadFinished);

    // Each time a block is written, update the progress bar
    connect(writer, &ImageWriter::blockWritten, this, &ImageWriterHelper::updateProgressBar);
    */

    ImageWriter* writer = new ImageWriter(zeroing ? "" : imageFile, selectedDevice);
    writer->writeImage();
    ActionReply reply; // success by default
    return reply;
}

KAUTH_HELPER_MAIN("org.kde.isoimagewriter", ImageWriterHelper)
