#ifndef COMMON_H
#define COMMON_H

////////////////////////////////////////////////////////////////////////////////
// This file contains some commonly-used constants and function declarations

#include <windows.h>

#include <QString>

// Default unit to be used when displaying file/device sizes (MB)
const quint64 DEFAULT_UNIT = 1048576;

// Application name used for titles in messageboxes
const QString ApplicationTitle = "ROSA Image Writer";

// Returns the number of blocks required to contain some number of bytes
// Input:
//  T      - any integer type
//  val    - number of bytes
//  factor - size of the block
// Returns:
//  the number of blocks of size <factor> required for <val> to fit in
template <typename T> T alignNumberDiv(T val, T factor)
{
    return ((val + factor - 1) / factor);
}

// Returns the total size of blocks required to contain some number of bytes
// Input:
//  T      - any integer type
//  val    - number of bytes
//  factor - size of the block
// Returns:
//  the total size of blocks of size <factor> required for <val> to fit in
template <typename T> T alignNumber(T val, T factor)
{
    return alignNumberDiv(val, factor) * factor;
}

// Converts the WinAPI error code into text message
// Input:
//  err - error code (GetLastError() is used by default)
// Returns:
//  text message in the current system language describing the error
QString errorMessageFromCode(DWORD err = GetLastError());

#endif // COMMON_H
