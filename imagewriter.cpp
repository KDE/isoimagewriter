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

void ImageWriter::writeImage()
{
    HANDLE imageFile = INVALID_HANDLE_VALUE;
    HANDLE deviceFile = INVALID_HANDLE_VALUE;
    HANDLE volume = INVALID_HANDLE_VALUE;
    const qint64 BLOCK_SIZE = 1024 * 1024;
    LPVOID buffer = NULL;
    bool isError = false;
    try
    {
        DWORD bret;

        buffer = VirtualAlloc(NULL, BLOCK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (buffer == NULL)
            throw "Failed to allocate memory for buffer.\nError code: " + QString::number(GetLastError());

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
            throw "Failed to open the image file.\nError code: " + QString::number(GetLastError());

        // Unmounting volumes that belong the selected device
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
                throw "Failed to open the drive " + m_Device->m_Volumes[i] + "\nError code: " + QString::number(GetLastError());
            if (!DeviceIoControl(volume, FSCTL_DISMOUNT_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
                throw "Failed to unmount the drive " + m_Device->m_Volumes[i] + "\nError code: " + QString::number(GetLastError());
            CloseHandle(volume);
            volume = INVALID_HANDLE_VALUE;
        }

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
            throw "Failed to open the target device.\nError code: " + QString::number(GetLastError());
        if (!DeviceIoControl(deviceFile, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
            throw "Failed to lock the target device.\nError code: " + QString::number(GetLastError());

        DISK_GEOMETRY dg;
        if (!DeviceIoControl(deviceFile, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &bret, NULL))
            throw "Failed to get the target device info.\nError code: " + QString::number(GetLastError());

        DWORD readBytes;
        DWORD writtenBytes;
        BOOL res;
        while ((res = ReadFile(imageFile, buffer, BLOCK_SIZE, &readBytes, NULL)) && (readBytes > 0))
        {
            // Align the number of bytes to the sector size
            readBytes = alignNumber(readBytes, dg.BytesPerSector);
            res = WriteFile(deviceFile, buffer, readBytes, &writtenBytes, NULL);
            if (!res)
                throw "Failed to write to the device:\nError code: " + QString::number(GetLastError());
            if (writtenBytes != readBytes)
                throw QString("The last block was not fully written (" + QString::number(writtenBytes) + " of " + QString::number(readBytes) + ")!\nAborting.");
            emit blockWritten(1);
            if (m_CancelWriting)
            {
                emit cancelled();
                break;
            }
        }
        if (!res)
            throw "Failed to read the image file:\nError code: " + QString::number(GetLastError());
    }
    catch (QString msg)
    {
        emit error(msg);
        isError = true;
    }

    if (volume != INVALID_HANDLE_VALUE)
        CloseHandle(volume);
    if (imageFile != INVALID_HANDLE_VALUE)
        CloseHandle(imageFile);
    if (deviceFile != INVALID_HANDLE_VALUE)
        CloseHandle(deviceFile);
    if (buffer != NULL)
        VirtualFree(buffer, BLOCK_SIZE, MEM_DECOMMIT | MEM_RELEASE);
    if (!isError && !m_CancelWriting)
        emit success();
    emit finished();
}

void ImageWriter::cancelWriting()
{
    m_Mutex.lock();
    m_CancelWriting = true;
    m_Mutex.unlock();
}
