/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

#include <QObject>
#include <QUrl>
#include <qqmlintegration.h>

class FileDialogBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit FileDialogBridge(QObject *parent = nullptr);

public slots:
    QUrl selectImageFile();

private:
    static const QString getImageFileFilter();
};
