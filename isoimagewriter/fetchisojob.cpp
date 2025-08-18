/*
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "fetchisojob.h"
#include <QDebug>
#include <QDir>
#include <QNetworkReply>
#include <QSharedPointer>
#include <QStandardPaths>

FetchIsoJob::FetchIsoJob(QObject *parent)
    : QObject(parent)
{
    m_network.setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);
    cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    QDir().mkpath(cache);
}

QNetworkReply *FetchIsoJob::downloadFile(const QUrl& url)
{
    auto file = QSharedPointer<QFile>::create(cache + '/' + url.fileName());
    if (!file->open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file to download to" << cache << url;
        return nullptr;
    }
    auto reply = m_network.get(QNetworkRequest(url));

    // Allow every redirect for now
    connect(reply, &QNetworkReply::redirected, reply, [reply] (const QUrl &url) {
        qDebug() << "redirecting to" << url << "from" << reply->url();
        reply->redirectAllowed();
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, file, reply] {
        if (this->m_canceled) {
                reply->abort();
                file->close();
                file->remove();
                qWarning() << "Stopped downloading"; 
                return;
            }
        file->write(reply->readAll());
    });
    connect(reply, &QNetworkReply::finished, this, [file, reply] {
        file->close();
        if (reply->error()) {
            file->remove();
            qWarning() << "Could not download" << reply->url() << reply->errorString();
        }
        reply->deleteLater();
        qDebug() << "done";
    });
    return reply;
}

void FetchIsoJob::fetch(const QUrl& url)
{
    auto reply = downloadFile(url);
    if (!reply) {
        Q_EMIT failed();
        return;
    }
    connect(reply, &QNetworkReply::downloadProgress, this, [this] (qint64 bytesReceived, qint64 bytesTotal) {
        if (bytesTotal == 0)
             return;
        Q_EMIT downloadProgressChanged(100 * bytesReceived / bytesTotal);
        qDebug() << "ISO download Progress " << 100*bytesReceived/ bytesTotal << "\n"; 
    });
    connect(reply, &QNetworkReply::finished, this, [reply, this, url] {
        if (reply->error()) {
            Q_EMIT failed();
        } else {
            Q_EMIT finished(cache + '/' + url.fileName());
        }
    });
    connect(reply, &QNetworkReply::redirected, reply, [this] (const QUrl &url) {
        m_fetchUrl = url;
    });

    const QString urlString = url.toString();
    downloadFile(QUrl(urlString + ".sig"));
    downloadFile(QUrl(urlString.left(urlString.length() - 4) + ".sha256sum"));

    m_fetchUrl = url;
}

#include "moc_fetchisojob.cpp"
