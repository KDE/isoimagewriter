#include "common.h"

////////////////////////////////////////////////////////////////////////////////
// Implementation of the non-template functions from common.h


// Converts the WinAPI and COM error code into text message
// Input:
//  prefixMessage - error description
//  errorCode     - error code (GetLastError() is used by default)
// Returns:
//  prefixMessage followed by a newline and the system error message for the errorCode
QString errorMessageFromCode(QString prefixMessage, DWORD errorCode)
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
        prefixMessage += "\n" + QString::fromWCharArray(msgBuffer);
        LocalFree(msgBuffer);
        return prefixMessage;
    }
    else
        return prefixMessage + "\n" + QObject::tr("Error code:") + " " + QString::number(errorCode);
}
