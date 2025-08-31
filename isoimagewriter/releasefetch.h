/*
    SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
    SPDX-License-Identifier: GPL-3.0-or-later
*/
#pragma once

#include <QCache>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QTimer>
#include <QUrl>

struct IsoRelease {
    QString name;
    QString version;
    QString url;
    QString sha256;
    QString filename;
    QString distro; // "Kubuntu", "Fedora", or "KDE neon"
};

class ReleaseFetch : public QObject
{
    Q_OBJECT

public:
    explicit ReleaseFetch(QObject *parent = nullptr);

    Q_INVOKABLE void fetchReleases();
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void clearCache();

Q_SIGNALS:
    void releasesReady(const QJsonArray &releases);
    void fetchFailed(const QString &error);
    void fetchProgress(const QString &status);

private slots:
    void onKubuntuMainPageFinished();
    void onKubuntuReleasePageFinished();
    void onKubuntuSha256Finished();
    void onFedoraReleasesFinished();
    void onTimeout();

private:
    QNetworkReply *fetchUrl(const QUrl &url);
    QStringList parseVersions(const QString &html);
    QString extractSHA256(const QString &sha256Content, const QString &filename);
    QJsonArray releasesToJson(const QList<IsoRelease> &releases);
    void fetchKubuntuReleases();
    void fetchFedoraReleases();
    void fetchKdeNeonReleases();
    void checkIfComplete();
    IsoRelease getLatestKubuntu();
    IsoRelease getLatestFedoraKDE();
    IsoRelease getLatestKdeNeon();

    QNetworkAccessManager m_network;
    QList<IsoRelease> m_kubuntuReleases;
    QList<IsoRelease> m_fedoraReleases;
    QList<IsoRelease> m_kdeNeonReleases;
    QStringList m_versions;
    int m_currentVersionIndex;
    bool m_canceled;
    bool m_kubuntuComplete;
    bool m_fedoraComplete;
    bool m_kdeNeonComplete;
    QString m_kubuntuBaseUrl;
    QTimer m_timeoutTimer;

    // Cache management
    static QCache<QString, QJsonArray> s_releaseCache;
};
