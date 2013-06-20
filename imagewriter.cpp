////////////////////////////////////////////////////////////////////////////////
// Implementation of ImageWriter


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
    const qint64 TRANSFER_BLOCK_SIZE = 1024 * 1024;
    void* buffer = NULL;

    bool isError = false;
    bool cancelRequested = false;

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
            throw formatErrorMessageFromCode(tr("Failed to allocate memory for buffer:"));
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
        buffer = malloc(TRANSFER_BLOCK_SIZE);
        if (buffer == NULL)
            throw tr("Failed to allocate memory for buffer.");
#endif

        // Open the source image file for reading
        QFile imageFile(m_ImageFile);
        if (!imageFile.open(QIODevice::ReadOnly))
            throw tr("Failed to open the image file:") + "\n" + imageFile.errorString();

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
                errMessages << formatErrorMessageFromCode(tr("Failed to open the drive") + " " + m_Device->m_Volumes[i]);
                continue;
            }
            // Trying to lock the volume but ignore if we failed (such call seems to be required for
            // dismounting the volume on WinXP)
            DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL);
            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
                errMessages << formatErrorMessageFromCode(tr("Failed to unmount the drive") + " " + m_Device->m_Volumes[i]);
            CloseHandle(volume);
            volume = INVALID_HANDLE_VALUE;
        }
#elif defined(Q_OS_MAC)
        struct statfs* mntEntries = NULL;
        int mntEntriesNum = getmntinfo(&mntEntries, MNT_WAIT);
        for (int i = 0; i < mntEntriesNum; ++i)
        {
            if (QString(mntEntries[i].f_mntfromname).startsWith(m_Device->m_PhysicalDevice))
            {
                // Mount point is the selected device or one of its partitions - try to unmount it
                if (unmount(mntEntries[i].f_mntonname, MNT_FORCE) != 0)
                    errMessages << tr("Failed to unmount the volume") + " " + m_Device->m_Volumes[i] + "\n" + strerror(errno);
            }
        }
#endif
        if (errMessages.size() > 0)
            throw errMessages.join("\n\n");

        // Open the target USB device for writing and lock it
        PhysicalDevice deviceFile(m_Device->m_PhysicalDevice);
        if (!deviceFile.open())
            throw tr("Failed to open the target device:") + "\n" + deviceFile.errorString();

        // The number of bytes to be written must be a multiple of sector size,
        // so first we get the sector size proper
        qint64 sectorSize = deviceFile.getDeviceSectorSize();
        if (sectorSize < 0)
            sectorSize = 512;

        qint64 readBytes;
        qint64 writtenBytes;
        // Start reading/writing cycle
        while ((readBytes = imageFile.read(static_cast<char*>(buffer), TRANSFER_BLOCK_SIZE)) && (readBytes > 0))
        {
            // Align the number of bytes to the sector size
            readBytes = alignNumber(readBytes, sectorSize);
            writtenBytes = deviceFile.write(static_cast<char*>(buffer), readBytes);
            if (writtenBytes < 0)
                throw tr("Failed to write to the device:") + "\n" + deviceFile.errorString();
            if (writtenBytes != readBytes)
                throw tr("The last block was not fully written (%1 of %2 bytes)!\nAborting.").arg(writtenBytes).arg(readBytes);

            // Inform the GUI thread that next block was written
            // TODO: Make sure that when TRANSFER_BLOCK_SIZE is not a multiple of DEFAULT_UNIT
            // this still works or at least fails compilation
            emit blockWritten(TRANSFER_BLOCK_SIZE / DEFAULT_UNIT);

            // Check for the cancel request (using temporary variable to avoid multiple unlock calls in the code)
            m_Mutex.lock();
            cancelRequested = m_CancelWriting;
            m_Mutex.unlock();
            if (cancelRequested)
            {
                // The cancel request was issued
                emit cancelled();
                break;
            }
        }
        if (readBytes < 0)
            throw tr("Failed to read the image file:") + "\n" + imageFile.errorString();
    }
    catch (QString msg)
    {
        // Something went wrong :-(
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
    if (!isError && !cancelRequested)
        emit success();

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
