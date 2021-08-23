/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

////////////////////////////////////////////////////////////////////////////////
// This file contains Linux implementation of platform-dependent functions

#include <KLocalizedString>

#include <QMessageBox>
#include <QDir>
#include <QRegularExpression>
#include <Solid/Block>
#include <Solid/Device>
#include <Solid/StorageDrive>

#include "mainapplication.h"
#include "usbdevice.h"


bool platformEnumFlashDevices(AddFlashDeviceCallbackProc callback, void* cbParam)
{
    const auto devices = Solid::Device::listFromType(Solid::DeviceInterface::StorageDrive);
    for (const auto &device : devices) {
        if (!device.is<Solid::StorageDrive>()) {
            qDebug() << "Ignoring" << device.displayName() << device.udi();
            continue;
        }

        auto storageDrive = device.as<Solid::StorageDrive>();
        auto block = device.as<Solid::Block>();
        if (storageDrive->isInUse()) {
            qDebug() << "Skipping" << device.displayName() << device.udi();
            continue;
        }

        UsbDevice* deviceData = new UsbDevice;
        deviceData->m_PhysicalDevice = block->device();
        deviceData->m_Volumes = QStringList{ deviceData->m_PhysicalDevice };

        const QString logicalBlockSizeFile = QStringLiteral("/sys/dev/block/%1:%2/queue/logical_block_size").arg(block->deviceMajor())
                                                                                                            .arg(block->deviceMinor());
        deviceData->m_SectorSize = readFileContents(logicalBlockSizeFile).toUInt();
        if (deviceData->m_SectorSize == 0)
            deviceData->m_SectorSize = 512;
        deviceData->m_Size = storageDrive->size();
        deviceData->m_VisibleName = device.displayName();

        callback(cbParam, deviceData);
    }
    return true;
}

bool ensureElevated()
{
    // on Linux we use KAuth which uses polkit to run necessary bits as root
    return true;
}
