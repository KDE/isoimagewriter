/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "filedialogbridge.h"
#include <QFileDialog>
#include <QStandardPaths>

FileDialogBridge::FileDialogBridge(QObject *parent)
    : QObject(parent)
{
}

QUrl FileDialogBridge::selectImageFile()
{
    QString fileName =
        QFileDialog::getOpenFileName(nullptr, tr("Select Image File"), QStandardPaths::writableLocation(QStandardPaths::HomeLocation), getImageFileFilter());

    return fileName.isEmpty() ? QUrl() : QUrl::fromLocalFile(fileName);
}

const QString FileDialogBridge::getImageFileFilter()
{
    return tr("Image Files (*.iso *.bin *.img *.iso.gz *.iso.xz *.img.zstd *.img.gz *.img.zx *.img.xz *.raw);;All Files (*)");
}