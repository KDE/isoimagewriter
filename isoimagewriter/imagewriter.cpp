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

////////////////////////////////////////////////////////////////////////////////
// Implementation of ImageWriter

#include <KLocalizedString>
#include <KAuth>

#include <QFile>

#include "common.h"
#include "imagewriter.h"
#include "physicaldevice.h"

ImageWriter::ImageWriter(const QString& ImageFile, UsbDevice* Device, QObject *parent) :
    QObject(parent),
    m_Device(Device),
    m_ImageFile(ImageFile),
    m_CancelWriting(false)
{
}

// The main method that writes the image
void ImageWriter::writeImage()
{
    qDebug() << "XX writeImage()";
    const qint64 TRANSFER_BLOCK_SIZE = 1024 * 1024;
    void* buffer = NULL;

    bool isError = false;
    bool cancelRequested = false;
    bool zeroing = (m_ImageFile == "");

    // Using try-catch for processing errors
    // Invalid values are used for indication non-initialized objects;
    // after the try-catch block all the initialized objects are freed
    try
    {
#if defined(Q_OS_WIN32)
        // Using VirtualAlloc so that the buffer was properly aligned (required for
        // direct access to devices and for unbuffered reading/writing)
        buffer = VirtualAlloc(NULL, TRANSFER_BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (buffer == NULL)
            throw formatErrorMessageFromCode(i18n("Failed to allocate memory for buffer:"));
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        buffer = malloc(TRANSFER_BLOCK_SIZE);
        if (buffer == NULL)
            throw i18n("Failed to allocate memory for buffer.");
#endif

        QFile imageFile;
        if (zeroing)
        {
            // Prepare zero-filled buffer
            memset(buffer, 0, TRANSFER_BLOCK_SIZE);
        }
        else
        {
            // Open the source image file for reading
            imageFile.setFileName(m_ImageFile);
            if (!imageFile.open(QIODevice::ReadOnly))
                throw i18n("Failed to open the image file: %1", imageFile.errorString());
        }

        // Unmount volumes that belong to the selected target device
        // TODO: Check first if they are used and show warning
        // (problem: have to show request in the GUI thread and return reply back here)
        QStringList errMessages;

#if defined(Q_OS_WIN32)
        for (int i = 0; i < m_Device->m_Volumes.size(); ++i)
        {
            DWORD bret;
            HANDLE volume = CreateFile(
                reinterpret_cast<const wchar_t*>(("\\\\.\\" + m_Device->m_Volumes[i]).utf16()),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );
            if (volume == INVALID_HANDLE_VALUE)
            {
                errMessages << formatErrorMessageFromCode(i18n("Failed to open the drive %1", m_Device->m_Volumes[i]));
                continue;
            }
            // Trying to lock the volume but ignore if we failed (such call seems to be required for
            // dismounting the volume on WinXP)
            DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL);
            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
                errMessages << formatErrorMessageFromCode(i18n("Failed to unmount the drive %1", m_Device->m_Volumes[i]));
            CloseHandle(volume);
            volume = INVALID_HANDLE_VALUE;
        }
#elif defined(Q_OS_MAC)
        struct statfs* mntEntries = NULL;
        int mntEntriesNum = getmntinfo(&mntEntries, MNT_WAIT);
        for (int i = 0; i < mntEntriesNum; ++i)
        {
            for (int j = 0; j < m_Device->m_Volumes.size(); ++j)
            {
                // Check that the mount point is either our target device itself or a partition on it
                if ((mntEntries[i].f_mntfromname == m_Device->m_Volumes[j]) ||
                    QString(mntEntries[i].f_mntfromname).startsWith(m_Device->m_Volumes[j] + 's'))
                {
                    // Mount point is the selected device or one of its partitions - try to unmount it
                    if (unmount(mntEntries[i].f_mntonname, MNT_FORCE) != 0)
                        errMessages << i18n("Failed to unmount the volume %1\n%2", m_Device->m_Volumes[i], strerror(errno));
                }
            }
        }
#endif
        if (errMessages.size() > 0)
            throw errMessages.join("\n\n");

        // Open the target USB device for writing and lock it
        PhysicalDevice deviceFile(m_Device->m_PhysicalDevice);
        if (!deviceFile.open())
            throw i18n("Failed to open the target device:\n%1", deviceFile.errorString());

        qint64 readBytes;
        qint64 writtenBytes;
        // Start reading/writing cycle
        for (;;)
        {
            qDebug() << "For Loop3";
            if (KAuth::HelperSupport::isStopped()) {
                qDebug() << "isStopped";
            } else {
                qDebug() << "not isStopped";
            }
            if (zeroing)
            {
                readBytes = TRANSFER_BLOCK_SIZE;
            }
            else
            {
                if ((readBytes = imageFile.read(static_cast<char*>(buffer), TRANSFER_BLOCK_SIZE)) <= 0)
                    break;
            }
            // Align the number of bytes to the sector size
            readBytes = alignNumber(readBytes, (qint64)m_Device->m_SectorSize);
            writtenBytes = deviceFile.write(static_cast<char*>(buffer), readBytes);
            if (writtenBytes < 0)
                throw i18n("Failed to write to the device:\n%1", deviceFile.errorString());
            if (writtenBytes != readBytes)
                throw i18n("The last block was not fully written (%1 of %2 bytes)!\nAborting.", writtenBytes, readBytes);

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            // In Linux/MacOS the USB device is opened with buffering. Using forced sync to validate progress bar.
            // For unknown reason, deviceFile.flush() does not work as intended here.
            fsync(deviceFile.handle());
#endif

            // Inform the GUI thread that next block was written
            // TODO: Make sure that when TRANSFER_BLOCK_SIZE is not a multiple of DEFAULT_UNIT
            // this still works or at least fails compilation
            emit blockWritten(TRANSFER_BLOCK_SIZE / DEFAULT_UNIT);
            KAuth::HelperSupport::progressStep(TRANSFER_BLOCK_SIZE / DEFAULT_UNIT); // FIXME why doesn't this work?
            QVariantMap progressArgs;
            progressArgs[QStringLiteral("progress")] = TRANSFER_BLOCK_SIZE / DEFAULT_UNIT;
            KAuth::HelperSupport::progressStep(progressArgs); // used because above doesn't work

            // Check for the cancel request (using temporary variable to avoid multiple unlock calls in the code)
            m_Mutex.lock();
            //cancelRequested = m_CancelWriting;
            cancelRequested = KAuth::HelperSupport::isStopped();
            m_Mutex.unlock();
            if (cancelRequested)
            {
                qDebug() << "cancelRequested";
                // The cancel request was issued
                emit cancelled();
                break;
            }
            if (zeroing)
            {
                // In zeroing mode only write 1 block - 1 MB is enough to clear both MBR and GPT
                break;
            }
        }
        if (!zeroing)
        {
            if (readBytes < 0)
                throw i18n("Failed to read the image file:\n%1", imageFile.errorString());
            imageFile.close();
        }
        deviceFile.close();
    }
    catch (QString msg)
    {
        // Something went wrong :-(
        QVariantMap args;
        args[QStringLiteral("error")] = msg;
        KAuth::HelperSupport::progressStep(args);
        emit error(msg);
        isError = true;
    }

    if (buffer != NULL)
#if defined(Q_OS_WIN32)
        VirtualFree(buffer, TRANSFER_BLOCK_SIZE, MEM_DECOMMIT | MEM_RELEASE);
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        free(buffer);
#endif

    // If no errors occurred and user did not stop the operation, it means everything went fine
    if (!isError && !cancelRequested) {
        QString message = i18n("The operation completed successfully.") +
            "<br><br>" +
            (zeroing ? i18n("Now you need to format your device.") : i18n("To be able to store data on this device again, please, use the button \"Wipe USB Disk\"."));
        QVariantMap args;
        args[QStringLiteral("success")] = message;
        KAuth::HelperSupport::progressStep(args);
        emit success(message);
    }

    // In any case the operation is finished
    emit finished();
}

// Implements reaction to the cancel request from user
void ImageWriter::cancelWriting()
{
    m_Mutex.lock();
    m_CancelWriting = true;
    m_Mutex.unlock();
}
