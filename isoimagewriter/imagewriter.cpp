/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Implementation of ImageWriter

#include "imagewriter.h"

#include <KLocalizedString>

#if defined(USE_KAUTH)
#include <kauth_version.h>
#if KAUTH_VERSION >= QT_VERSION_CHECK(5, 92, 0)
#include <KAuth/ActionReply>
#include <KAuth/HelperSupport>
#else
#include <KAuth>
#endif
#endif

#include <QtDBus/QtDBus>
#include <QFile>
#include <KCompressionDevice>

#include <fcntl.h>
#include <unistd.h>

typedef QHash<QString, QVariant> Properties;
typedef QHash<QString, Properties> InterfacesAndProperties;
typedef QHash<QDBusObjectPath, InterfacesAndProperties> DBusIntrospection;
Q_DECLARE_METATYPE(Properties)
Q_DECLARE_METATYPE(InterfacesAndProperties)
Q_DECLARE_METATYPE(DBusIntrospection)

#include "common.h"
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

        QIODevice* device;
        if (imageFile.fileName().endsWith(".gz")) {
            device = new KCompressionDevice(&imageFile, true, KCompressionDevice::GZip);
            if (!device->open(QIODevice::ReadOnly)) {
                throw i18n("Failed to open compression device: %1", device->errorString());
            }
        } else if (imageFile.fileName().endsWith(".xz")) {
            device = new KCompressionDevice(&imageFile, true, KCompressionDevice::Xz);
            if (!device->open(QIODevice::ReadOnly)) {
                throw i18n("Failed to open compression device: %1", device->errorString());
            }
        } else if (imageFile.fileName().endsWith(".zstd")) {
            device = new KCompressionDevice(&imageFile, true, KCompressionDevice::Zstd);
            if (!device->open(QIODevice::ReadOnly)) {
                throw i18n("Failed to open compression device: %1", device->errorString());
            }
        } else {
            device = &imageFile;
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
        /*
        PhysicalDevice deviceFile(m_Device->m_PhysicalDevice);
        qDebug() << "writeImage() opening m_PhysicalDevice: " << m_Device->m_PhysicalDevice;
        if (!deviceFile.open())
            throw i18n("Failed to open the target device:\n%1", deviceFile.errorString());
        */
        // temperarily? calling udisks locally to get a file descriptor and pass that to QFile to open for writing
        // not working so try to copy whatever mediawriter/helper/write.cpp WriteJob::writePlain(int fd) does
        QDBusInterface deviceDBus("org.freedesktop.UDisks2", m_Device->m_PhysicalDevice, "org.freedesktop.UDisks2.Block", QDBusConnection::systemBus(), this);
        QDBusReply<QDBusUnixFileDescriptor> reply = deviceDBus.call(QDBus::Block, "OpenDevice", "rw", Properties{{"flags", O_DIRECT | O_SYNC | O_CLOEXEC}} );
        QDBusUnixFileDescriptor fd = reply.value();
        //QFile deviceFile;
        //deviceFile.open(fd.fileDescriptor(), QIODevice::WriteOnly);

        qint64 readBytes;
        qint64 writtenBytes;
        // Start reading/writing cycle
        for (;;)
        {
            qDebug() << "For Loop3";
#if defined(USE_KAUTH)
            if (KAuth::HelperSupport::isStopped()) {
                qDebug() << "isStopped";
            } else {
                qDebug() << "not isStopped";
            }
#endif
            if (zeroing)
            {
                readBytes = TRANSFER_BLOCK_SIZE;
            }
            else
            {
                if ((readBytes = device->read(static_cast<char*>(buffer), TRANSFER_BLOCK_SIZE)) <= 0)
                    break;
            }
            // Align the number of bytes to the sector size
            //writtenBytes = deviceFile.write(static_cast<char*>(buffer), readBytes);
            readBytes = alignNumber(readBytes, (qint64)m_Device->m_SectorSize);
            writtenBytes = ::write(fd.fileDescriptor(), buffer, readBytes);
            //if (writtenBytes != readBytes) {
            if (writtenBytes < 0) {
                qDebug() << "write writtenBytes: " << writtenBytes;
                //throw i18n("Failed to write to the device:\n%1"); //, "ook"); //deviceFile.errorString());
            }
            if (writtenBytes != readBytes) {
                throw i18n("The last block was not fully written (%1 of %2 bytes)!\nAborting.", writtenBytes, readBytes);
            }

#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
            // In Linux/MacOS the USB device is opened with buffering. Using forced sync to validate progress bar.
            // For unknown reason, deviceFile.flush() does not work as intended here.
            //fsync(deviceFile.handle());
#endif
            const int percent = (100 * imageFile.pos()) / imageFile.size();
            // Inform the GUI thread that next block was written
            // TODO: Make sure that when TRANSFER_BLOCK_SIZE is not a multiple of DEFAULT_UNIT
            // this still works or at least fails compilation
            emit progressChanged(percent);

#if defined(USE_KAUTH)
            KAuth::HelperSupport::progressStep(percent);
#endif

            // Check for the cancel request (using temporary variable to avoid multiple unlock calls in the code)
            m_Mutex.lock();
#if defined(USE_KAUTH)
            cancelRequested = KAuth::HelperSupport::isStopped();
#else
            cancelRequested = m_CancelWriting;
#endif
            m_Mutex.unlock();

            if (cancelRequested)
            {
#if defined(USE_KAUTH)
                QVariantMap progressArgs;
                progressArgs[QStringLiteral("cancel")] = true;
                KAuth::HelperSupport::progressStep(progressArgs);
#endif

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
            if (readBytes < 0) {
                throw i18n("Failed to read the image file:\n%1", device->errorString());
            }
            imageFile.close();
        }
        //deviceFile.close();
    }
    catch (QString msg)
    {
        // Something went wrong :-(
#if defined(USE_KAUTH)
        QVariantMap args;
        args[QStringLiteral("error")] = msg;
        KAuth::HelperSupport::progressStep(args);
#endif

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

#if defined(USE_KAUTH)
        QVariantMap args;
        args[QStringLiteral("success")] = message;
        KAuth::HelperSupport::progressStep(args);
#endif

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
