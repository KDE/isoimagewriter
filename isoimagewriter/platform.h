/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtGlobal>

#if defined(Q_OS_WIN32)
#include <Shobjidl.h>
#include <Wbemidl.h>
#include <WinIoCtl.h>
#include <comutil.h>
#include <dbt.h>
#include <io.h>
#include <windows.h>
#endif

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
#include <unistd.h>
#endif

#if defined(Q_OS_MAC)
#include <errno.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <unistd.h>
#endif

#endif // PLATFORM_H
