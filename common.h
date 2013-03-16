#ifndef COMMON_H
#define COMMON_H

#include <windows.h>

#include <QString>

const quint64 DEFAULT_UNIT = 1048576;
const QString ApplicationTitle = "ROSA Image Writer";

template <typename T> T alignNumberDiv(T val, T factor)
{
    return ((val + factor - 1) / factor);
}

template <typename T> T alignNumber(T val, T factor)
{
    return alignNumberDiv(val, factor) * factor;
}

QString errorMessageFromCode(DWORD err = GetLastError());

#endif // COMMON_H
