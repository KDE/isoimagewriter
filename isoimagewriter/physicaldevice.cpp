/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// Implementation of PhysicalDevice

#include <KLocalizedString>

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
        setErrorString(formatErrorMessageFromCode(i18n("Could not acquire lock:")));
        return false;
    }
    // Construct QFile around the device handle; close() will now close the handle automatically
    if (QFile::open(_open_osfhandle(reinterpret_cast<intptr_t>(m_fileHandle), 0), QIODevice::WriteOnly | QIODevice::Unbuffered, AutoCloseHandle))
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
