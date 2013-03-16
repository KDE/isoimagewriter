#ifndef COMMON_H
#define COMMON_H

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

#endif // COMMON_H
