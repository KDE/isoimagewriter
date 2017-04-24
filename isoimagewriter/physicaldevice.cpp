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
