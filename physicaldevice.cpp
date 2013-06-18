////////////////////////////////////////////////////////////////////////////////
// Implementation of PhysicalDevice


#include "physicaldevice.h"

PhysicalDevice::PhysicalDevice(const QString& name) :
    QFile(name)
{
}

// Opens the selected device in WriteOnly mode
bool PhysicalDevice::open()
{
#if defined(Q_OS_WIN32)
    DWORD bret;

    // In Windows QFile with write mode uses disposition OPEN_ALWAYS, but WinAPI
    // requires OPEN_EXISTING for physical devices. Therefore we have to use native API.
    m_fileHandle = CreateFile(
        reinterpret_cast<const wchar_t*>(fileName().utf16()),
        GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING,
        NULL
    );
    if (m_fileHandle == INVALID_HANDLE_VALUE)
    {
        setErrorString(errorMessageFromCode());
        return false;
    }
    // Lock the opened device
    if (!DeviceIoControl(m_fileHandle, FSCTL_LOCK_VOLUME, NULL, 0, NULL, 0, &bret, NULL))
    {
        setErrorString(formatErrorMessageFromCode(tr("Could not acquire lock:")));
        return false;
    }
    // Construct QFile around the device handle; close() will now close the handle automatically
    if (QFile::open(_open_osfhandle(reinterpret_cast<intptr_t>(m_fileHandle), 0), QIODevice::WriteOnly))
        return true;
    else
    {
        CloseHandle(m_fileHandle);
        return false;
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    // Simply use QFile, it works fine in Linux
    // TODO: Use system call open with O_DIRECT
    return QFile::open(QIODevice::WriteOnly);
#else
    return false;
#endif
}

// Gets the the device's sector size
int PhysicalDevice::getDeviceSectorSize()
{
    if (!isOpen())
    {
        setErrorString(tr("The device is not open."));
        return -1;
    }

    // Use platform-specific IOCTLs for getting the sector size
#if defined(Q_OS_WIN32)
    DWORD bret;
    DISK_GEOMETRY dg;
    if (DeviceIoControl(m_fileHandle, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &dg, sizeof(dg), &bret, NULL))
        return dg.BytesPerSector;
#elif defined(Q_OS_LINUX)
    int sectorSize;
    if (ioctl(handle(), BLKSSZGET, &sectorSize) == 0)
        return sectorSize;
#endif

    // If failed for some reason (or unsupported platform), default is 512 bytes
    return 512;
}
