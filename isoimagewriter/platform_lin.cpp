/*
    SPDX-FileCopyrightText: 2016 ROSA
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// This file contains Linux implementation of platform-dependent functions

#include <KLocalizedString>

#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>

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
    // on Linux we use KAuth which uses polkit to run necessary bits as root
    return true;
}
