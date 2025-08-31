/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef FILEDIALOGBRIDGE_H
#define FILEDIALOGBRIDGE_H

#include <QObject>
#include <QUrl>

class FileDialogBridge : public QObject
{
    Q_OBJECT

public:
    explicit FileDialogBridge(QObject *parent = nullptr);

public slots:
    QUrl selectImageFile();

private:
    static const QString getImageFileFilter();
};

#endif // FILEDIALOGBRIDGE_H