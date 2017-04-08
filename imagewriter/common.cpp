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

#include "common.h"

#include <KLocalizedString>

#include <QFile>
#include <QStringList>
////////////////////////////////////////////////////////////////////////////////
// Implementation of the non-template cross-platform functions from common.h


#if defined(Q_OS_WIN32)
// Converts the WinAPI and COM error code into text message
// Input:
//  errorCode - error code (GetLastError() is used by default)
// Returns:
//  system error message for the errorCode
QString errorMessageFromCode(DWORD errorCode)
{
    LPTSTR msgBuffer;
    DWORD res = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorCode,
        0,
        reinterpret_cast<LPTSTR>(&msgBuffer),
        0,
        NULL
    );
    if (res)
    {
        QString ret = QString::fromWCharArray(msgBuffer);
        LocalFree(msgBuffer);
        return ret;
    }
    else
        return i18n("Error code: %1", QString::number(errorCode));
}

// Converts the WinAPI and COM error code into text message
// Input:
//  prefixMessage - error description
//  errorCode     - error code (GetLastError() is used by default)
// Returns:
//  prefixMessage followed by a newline and the system error message for the errorCode
QString formatErrorMessageFromCode(QString prefixMessage, DWORD errorCode)
{
    return prefixMessage + "\n" + errorMessageFromCode(errorCode);
}

// This constant is declared in wbemprov.h and defined in wbemuuid.lib. If building with MinGW, the header is available but not library,
// and the constant remains unresolved. So we define it here.
const CLSID CLSID_WbemAdministrativeLocator = {0xCB8555CC, 0x9128, 0x11D1, {0xAD, 0x9B, 0x00, 0xC0, 0x4F, 0xD8, 0xFD, 0xFF}};
#endif

// Gets the contents of the specified file
// Input:
//  fileName - path to the file to read
// Returns:
//  the file contents or empty string if an error occurred
QString readFileContents(const QString& fileName)
{
    QFile f(fileName);
    if (!f.open(QFile::ReadOnly))
        return "";
    QString ret = f.readAll();
    f.close();
    return ret;
}
