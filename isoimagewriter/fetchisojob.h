/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FETCHISOJOB_H
#define FETCHISOJOB_H

#include <QNetworkAccessManager>

class FetchIsoJob : public QObject
{
    Q_OBJECT
public:
    FetchIsoJob(QObject *parent);

    void fetch(const QUrl &url);
    QUrl fetchUrl() const {
        return m_fetchUrl;
    }

Q_SIGNALS:
    void failed();
    void finished(const QString &destination);
    void downloadProgressChanged(int percentage);

private:
    QNetworkReply *downloadFile(const QUrl &url);

    QNetworkAccessManager m_network;
    QUrl m_fetchUrl;
    QString cache;
};

#endif // FETCHISOJOB_H
