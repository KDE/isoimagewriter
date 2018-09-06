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
