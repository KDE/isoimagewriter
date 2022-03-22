/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtGlobal>

#if defined(Q_OS_WIN32)
#include <windows.h>
#include <WinIoCtl.h>
#include <dbt.h>
#include <Wbemidl.h>
#include <Shobjidl.h>
#include <comutil.h>
#include <io.h>
#endif

#if defined(Q_OS_LINUX)
#include <unistd.h>
#endif

#if defined(Q_OS_MAC)
#include <unistd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <errno.h>
#endif

#endif // PLATFORM_H
