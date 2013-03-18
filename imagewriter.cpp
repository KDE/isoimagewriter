////////////////////////////////////////////////////////////////////////////////
// Implementation of ImageWriter


#include <windows.h>

#include <QFile>

#include "common.h"
#include "imagewriter.h"

ImageWriter::ImageWriter(const QString& ImageFile, UsbDevice* Device, QObject *parent) :
    QObject(parent),
    m_CancelWriting(false),
    m_ImageFile(ImageFile),
    m_Device(Device)
{
}

// The main method that writes the image
void ImageWriter::writeImage()
{
    // Using try-catch for processing errors
    // Invalid values are used for indication non-initialized objects;
    // after the try-catch block all the initialized objects are freed
    HANDLE imageFile = INVALID_HANDLE_VALUE;
    HANDLE deviceFile = INVALID_HANDLE_VALUE;
    HANDLE volume = INVALID_HANDLE_VALUE;
    LPVOID buffer = NULL;

    const qint64 BLOCK_SIZE = 1024 * 1024;
    bool isError = false;
    bool cancelRequested = false;

    try
    {
        DWORD bret;

        // Using VirtualAlloc so that the buffer was properly aligned (required for
        // direct access to devices and for unbuffered reading/writing)
        buffer = VirtualAlloc(NULL, BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (buffer == NULL)
            throw errorMessageFromCode("Failed to allocate memory for buffer:");

        // Open the source image file for reading
        imageFile = CreateFile(
            reinterpret_cast<const wchar_t*>(m_ImageFile.utf16()),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
        );
        if (imageFile == INVALID_HANDLE_VALUE)
            throw errorMessageFromCode("Failed to open the image file:");

        // Unmount volumes that belong to the selected target device
        // TODO: Check first if they are used and show warning
        // (problem: have to show request in the GUI thread and return reply back here)
        for (int i = 0; i < m_Device->m_Volumes.size(); ++i)
        {
            volume = CreateFile(
                reinterpret_cast<const wchar_t*>(("\\\\.\\" + m_Device->m_Volumes[i]).utf16()),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
            );
            if (volume == INVALID_HANDLE_VALUE)
                throw errorMessageFromCode("Failed to open the drive " + m_Device->m_Volumes[i]);
            // Trying to lock the volume but ignore if we failed (such call seems to be required for
            // dismounting the volume on WinXP)
            DeviceIoControl(volume, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL);
            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
                throw errorMessageFromCode("Failed to unmount the drive " + m_Device->m_Volumes[i]);
            CloseHandle(volume);
            volume = INVALID_HANDLE_VALUE;
        }

        // Open the target USB device for writing and lock it
        deviceFile = CreateFile(
            reinterpret_cast<const wchar_t*>(m_Device->m_PhysicalDevice.utf16()),
            GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
            NULL
        );
        if (deviceFile == INVALID_HANDLE_VALUE)
            throw errorMessageFromCode("Failed to open the target device:");
        if (!DeviceIoControl(deviceFile, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
            throw errorMessageFromCode("Failed to lock the target device:");

        // The number of bytes to be written must be a multiple of sector size,
        // so first we get the sector size proper (DISK_GEOMETRY::BytesPerSector)
        DISK_GEOMETRY dg;
        if (!DeviceIoControl(deviceFile, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &bret, NULL))
            throw errorMessageFromCode("Failed to get the target device info:");

        DWORD readBytes;
        DWORD writtenBytes;
        BOOL res;
        // Start reading/writing cycle
        while ((res = ReadFile(imageFile, buffer, BLOCK_SIZE, &readBytes, NULL)) && (readBytes > 0))
        {
            // Align the number of bytes to the sector size
            readBytes = alignNumber(readBytes, dg.BytesPerSector);
            res = WriteFile(deviceFile, buffer, readBytes, &writtenBytes, NULL);
            if (!res)
                throw errorMessageFromCode("Failed to write to the device:");
            if (writtenBytes != readBytes)
                throw QString("The last block was not fully written (" + QString::number(writtenBytes) + " of " + QString::number(readBytes) + ")!\nAborting.");

            // Inform the GUI thread that next block was written
            // TODO: Make sure that when BLOCK_SIZE is not a multiple of DEFAULT_UNIT this still
            // works or at least fails compilation
            emit blockWritten(BLOCK_SIZE / DEFAULT_UNIT);

            // Check for the cancel request (using temporary variable to avoid multiple unlocks in the code)
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
        if (!res)
            throw errorMessageFromCode("Failed to read the image file:");
    }
    catch (QString msg)
    {
        // Something went wrong :-(
        emit error(msg);
        isError = true;
    }

    // The cleanup stage
    if (volume != INVALID_HANDLE_VALUE)
        CloseHandle(volume);
    if (imageFile != INVALID_HANDLE_VALUE)
        CloseHandle(imageFile);
    if (deviceFile != INVALID_HANDLE_VALUE)
        CloseHandle(deviceFile);
    if (buffer != NULL)
        VirtualFree(buffer, BLOCK_SIZE, MEM_DECOMMIT | MEM_RELEASE);

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
