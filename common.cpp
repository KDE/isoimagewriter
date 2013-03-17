#include "common.h"

////////////////////////////////////////////////////////////////////////////////
// Implementation of the non-template functions from common.h


// Converts the WinAPI error code into text message
// Input:
//  err - error code (GetLastError() is used by default)
// Returns:
//  text message in the current system language describing the error
QString errorMessageFromCode(DWORD err)
{
    LPTSTR msgBuffer;
    DWORD res = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        0,
        (LPTSTR)&msgBuffer,
        0,
        NULL
    );
    if (res)
    {
        QString msg = QString::fromWCharArray(msgBuffer);
        LocalFree(msgBuffer);
        return msg;
    }
    else
        return "Error code: " + QString::number(err);
}
