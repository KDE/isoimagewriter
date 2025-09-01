/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

////////////////////////////////////////////////////////////////////////////////
// ImageWriter is a class for writing image file to the USB flash disk

#include <QMutex>
#include <QObject>

#include "usbdevice.h"

class ImageWriter : public QObject
{
    Q_OBJECT

public:
    explicit ImageWriter(const QString &ImageFile, UsbDevice *Device, QObject *parent = nullptr);

protected:
    // Information about the selected USB flash disk
    UsbDevice *m_Device;
    // Source image file (full path); if empty, zero-filled buffer of 1 MB is used
    QString m_ImageFile;
    // Flag used for cancelling the operation by user
    bool m_CancelWriting;
    // Mutex for synchronizing access to m_CancelWriting member
    QMutex m_Mutex;

signals:
    // Emitted when writeImage is finished for any reason
    void finished();
    // Emitted on successful completion
    void success(QString msg);
    // Emitted when something wrong happened, <msg> is the error message
    void error(QString msg);
    // Emitted when processed the cancel request from user
    void cancelled();
    // Emitted as the progress percentage changes
    void progressChanged(int percent);

public slots:
    // The main method that writes the image
    void writeImage();
    // Implements reaction to the cancel request from user
    void cancelWriting();
};


