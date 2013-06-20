#ifndef PLATFORM_H
#define PLATFORM_H

#include <QtGlobal>

#if defined(Q_OS_WIN32)
#include <windows.h>
#include <dbt.h>
#include <Wbemidl.h>
#include <Shobjidl.h>
#include <comutil.h>
#include <io.h>
#endif

#if defined(Q_OS_LINUX)
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <libudev.h>
#endif

#if defined(Q_OS_MAC)
#include <unistd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#include <errno.h>
#endif

#endif // PLATFORM_H
