/*
 * Copyright 2016 ROSA
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

#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

////////////////////////////////////////////////////////////////////////////////
// ImageWriter is a class for writing image file to the USB flash disk


#include <QObject>
#include <QMutex>

#include "usbdevice.h"

class ImageWriter : public QObject
{
    Q_OBJECT

public:
    explicit ImageWriter(const QString& ImageFile, UsbDevice* Device, QObject *parent = nullptr);

protected:
    // Information about the selected USB flash disk
    UsbDevice* m_Device;
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
    // Emitted each time a block is written, <count> is a number of DEFAULT_UNIT-size blocks
    void blockWritten(int count);

public slots:
    // The main method that writes the image
    void writeImage();
    // Implements reaction to the cancel request from user
    void cancelWriting();
};

#endif // IMAGEWRITER_H
