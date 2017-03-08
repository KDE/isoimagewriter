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

////////////////////////////////////////////////////////////////////////////////
// This file contains Linux implementation of platform-dependent functions

#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>

#include "platform_lin_suprogram.h"
#include "mainapplication.h"
#include "usbdevice.h"


bool platformEnumFlashDevices(AddFlashDeviceCallbackProc callback, void* cbParam)
{
    // Using /sys/bus/usb/devices directory contents for enumerating the USB devices
    //
    // Details:
    // Take the devices which have <device>/bInterfaceClass contents set to "08" (storage device).
    //
    // 1. To get the user-friendly name we need to read the <manufacturer> and <product> files
    //    of the parent device (the parent device is the one with less-specified name, e.g. "2-1" for "2-1:1.0").
    //
    // 2. The block device name can be found by searching the contents of the following subdirectory:
    //      <device>/host*/target*/<scsi-device-name>/block/
    //    where * is a placeholder, and <scsi-device-name> starts with the same substring that "target*" ends with.
    //    For example, this path may look like follows:
    //      /sys/bus/usb/devices/1-1:1.0/host4/target4:0:0/4:0:0:0/block/
    //    This path contains the list of block devices by their names, e.g. sdc, which gives us /dev/sdc.
    //
    // 3. And, finally, for the device size we multiply .../block/sdX/size (the number of sectors) with
    //    .../block/sdX/queue/logical_block_size (the sector size).

    // Start with enumerating all the USB devices
    QString usbDevicesRoot = "/sys/bus/usb/devices";
    QDir dirList(usbDevicesRoot);
    QStringList usbDevices = dirList.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int deviceIdx = 0; deviceIdx < usbDevices.size(); ++deviceIdx)
    {
        QDir deviceDir = dirList;
        if (!deviceDir.cd(usbDevices[deviceIdx]))
            continue;

        // Skip devices with wrong interface class
        if (readFileContents(deviceDir.absoluteFilePath("bInterfaceClass")) != "08\n")
            continue;

        // Search for "host*" entries and process them
        QStringList hosts = deviceDir.entryList(QStringList("host*"));
        for (int hostIdx = 0; hostIdx < hosts.size(); ++hostIdx)
        {
            QDir hostDir = deviceDir;
            if (!hostDir.cd(hosts[hostIdx]))
                continue;

            // Search for "target*" entries and process them
            QStringList targets = hostDir.entryList(QStringList("target*"));
            for (int targetIdx = 0; targetIdx < targets.size(); ++targetIdx)
            {
                QDir targetDir = hostDir;
                if (!targetDir.cd(targets[targetIdx]))
                    continue;

                // Remove the "target" part and append "*" to search for appropriate SCSI devices
                QStringList scsiTargets = targetDir.entryList(QStringList(targets[targetIdx].mid(6) + "*"));
                for (int scsiTargetIdx = 0; scsiTargetIdx < scsiTargets.size(); ++scsiTargetIdx)
                {
                    QDir scsiTargetDir = targetDir;
                    if (!scsiTargetDir.cd(scsiTargets[scsiTargetIdx] + "/block"))
                        continue;

                    // Read the list of block devices and process them
                    QStringList blockDevices = scsiTargetDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
                    for (int blockDeviceIdx = 0; blockDeviceIdx < blockDevices.size(); ++blockDeviceIdx)
                    {
                        // Create the new UsbDevice object to bind information to the listbox entry
                        UsbDevice* deviceData = new UsbDevice;

                        // Use the block device name as both physical device and displayed volume name
                        deviceData->m_PhysicalDevice = "/dev/" + blockDevices[blockDeviceIdx];
                        deviceData->m_Volumes << deviceData->m_PhysicalDevice;

                        // Get the device size
                        quint64 blocksNum = readFileContents(scsiTargetDir.absoluteFilePath(blockDevices[blockDeviceIdx] + "/size")).toULongLong();
                        // The size is counted in logical blocks (tested with 4K-sector HDD)
                        deviceData->m_SectorSize = readFileContents(scsiTargetDir.absoluteFilePath(blockDevices[blockDeviceIdx] + "/queue/logical_block_size")).toUInt();
                        if (deviceData->m_SectorSize == 0)
                            deviceData->m_SectorSize = 512;
                        deviceData->m_Size = blocksNum * deviceData->m_SectorSize;

                        // Get the user-friendly name for the device by reading the parent device fields
                        QString usbParentDevice = usbDevices[deviceIdx];
                        usbParentDevice.replace(QRegularExpression("^(\\d+-\\d+):.*$"), "\\1");
                        usbParentDevice.prepend(usbDevicesRoot + "/");
                        // TODO: Find out how to get more "friendly" name (for SATA-USB connector it shows the bridge
                        // device name instead of the disk drive name)
                        deviceData->m_VisibleName = (
                                readFileContents(usbParentDevice + "/manufacturer").trimmed() +
                                " " +
                                readFileContents(usbParentDevice + "/product").trimmed()
                            ).trimmed();

                        // The device information is now complete, append the entry
                        callback(cbParam, deviceData);
                    }
                }
            }
        }
    }

    return true;
}

bool ensureElevated()
{
    // If we already have root privileges do nothing
    uid_t uid = getuid();
    if (uid == 0)
        return true;

    // Search for known GUI su-applications.
    // The list is priority-ordered. If there are native su-applications present,
    // using the first such program. Otherwise, using just the first program that is present.
    QList<SuProgram*> suPrograms = { new XdgSu(), new BeeSu(), new KdeSu(), new GkSu() };
    SuProgram* suProgram = NULL;
    for (int i = 0; i < suPrograms.size(); ++i)
    {
        // Skip missing su-apps
        if (!suPrograms[i]->isPresent())
            continue;

        if (suPrograms[i]->isNative())
        {
            // If we found a native su-application - using it as preferred and stop searching
            suProgram = suPrograms[i];
            break;
        }
        else
        {
            // If not native, and no other su-application was found - using it, but continue searching,
            // in case a native app will appear down the list
            if (suProgram == NULL)
                suProgram = suPrograms[i];
        }
    }
    if (suProgram == NULL)
    {
        QMessageBox::critical(
            NULL,
            ApplicationTitle,
            "<font color=\"red\">" + QObject::tr("Error!") + "</font> " + QObject::tr("No appropriate su-application found!") + "<br>" +
            QObject::tr("Please, restart the program with root privileges."),
            QMessageBox::Ok
        );
        return false;
    }

    // Prepare the list of arguments and restart ourselves using the su-application found
    QStringList args;
    // First comes our own executable
    args << mApp->applicationFilePath();
    // We need to explicitly pass language and initial directory so that the new instance
    // inherited the current user's parameters rather than root's
    QString argLang = mApp->getLocale();
    if (!argLang.isEmpty())
        args << "--lang=" + argLang;
    QString argDir = mApp->getInitialDir();
    if (!argDir.isEmpty())
        args << "--dir=" + argDir;
    // Finally, if image file was supplied, append it as well
    QString argImage = mApp->getInitialImage();
    if (!argImage.isEmpty())
        args << argImage;
    // And now try to take off with all this garbage
    suProgram->restartAsRoot(args);

    // Something went wrong, we should have never returned! Cleanup and return error
    for (int i = 0; i < suPrograms.size(); ++i)
        delete suPrograms[i];
    QMessageBox::critical(
        NULL,
        ApplicationTitle,
        "<font color=\"red\">" + QObject::tr("Error!") + "</font> " + QObject::tr("Failed to restart with root privileges! (Error code: %1)").arg(errno) + "<br>" +
        QObject::tr("Please, restart the program with root privileges."),
        QMessageBox::Ok
    );
    return false;
}
