/*
    SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <algorithm>

#include "releasefetch.h"

#define FEDORA_URL "https://fedoraproject.org//releases.json"
#define KUBUNTU_URL "https://cdimage.ubuntu.com/kubuntu/releases/"
#define KDENEON_URL "https://files.kde.org/neon/images/user/current/"

// Static cache initialization
QCache<QString, QJsonArray> ReleaseFetch::s_releaseCache(10); // Cache up to 10 entries

ReleaseFetch::ReleaseFetch(QObject *parent)
    : QObject(parent)
    , m_currentVersionIndex(0)
    , m_canceled(false)
    , m_kubuntuComplete(false)
    , m_fedoraComplete(false)
    , m_kdeNeonComplete(false)
    , m_kubuntuBaseUrl(KUBUNTU_URL)
{
    m_network.setRedirectPolicy(QNetworkRequest::UserVerifiedRedirectPolicy);

    // Setup timeout timer
    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.setInterval(10000); // 10 seconds
    connect(&m_timeoutTimer, &QTimer::timeout, this, &ReleaseFetch::onTimeout);
}

void ReleaseFetch::fetchReleases()
{
    // Check cache first
    const QString cacheKey = "latest_releases";
    QJsonArray *cachedReleases = s_releaseCache.object(cacheKey);

    if (cachedReleases) {
        emit fetchProgress("Using cached releases...");
        emit releasesReady(*cachedReleases);
        return;
    }

    // Not in cache, fetch fresh data
    m_canceled = false;
    m_kubuntuReleases.clear();
    m_fedoraReleases.clear();
    m_kdeNeonReleases.clear();
    m_versions.clear();
    m_currentVersionIndex = 0;
    m_kubuntuComplete = false;
    m_fedoraComplete = false;
    m_kdeNeonComplete = false;

    emit fetchProgress("Starting release fetch...");

    // Start the 10-second timeout timer
    m_timeoutTimer.start();

    // Start both fetches in parallel
    fetchKubuntuReleases();
    fetchFedoraReleases();
    fetchKdeNeonReleases();
}

void ReleaseFetch::fetchKubuntuReleases()
{
    emit fetchProgress("Fetching Kubuntu releases...");

    // Fetch main directory listing
    QUrl url(m_kubuntuBaseUrl);
    QNetworkReply *reply = fetchUrl(url);
    if (reply) {
        connect(reply, &QNetworkReply::finished, this, &ReleaseFetch::onKubuntuMainPageFinished);
    } else {
        emit fetchFailed("Failed to start Kubuntu network request");
    }
}

void ReleaseFetch::fetchFedoraReleases()
{
    emit fetchProgress("Fetching Fedora releases...");

    QUrl url("https://fedoraproject.org/releases.json");
    QNetworkReply *reply = fetchUrl(url);
    if (reply) {
        connect(reply, &QNetworkReply::finished, this, &ReleaseFetch::onFedoraReleasesFinished);
    } else {
        emit fetchFailed("Failed to start Fedora network request");
    }
}

void ReleaseFetch::cancel()
{
    m_canceled = true;
    m_timeoutTimer.stop();
}

void ReleaseFetch::clearCache()
{
    s_releaseCache.clear();
    emit fetchProgress("Cache cleared");
}

QNetworkReply *ReleaseFetch::fetchUrl(const QUrl &url)
{
    if (m_canceled) {
        return nullptr;
    }

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "KubuntuFetcher/1.0");
    QNetworkReply *reply = m_network.get(request);

    return reply;
}

void ReleaseFetch::onKubuntuMainPageFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (m_canceled) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchFailed("Failed to fetch Kubuntu releases page: " + reply->errorString());
        return;
    }

    QString html = QString::fromUtf8(reply->readAll());
    m_versions = parseVersions(html);

    if (m_versions.isEmpty()) {
        emit fetchFailed("No Kubuntu versions found");
        return;
    }

    // Start fetching individual release pages
    m_currentVersionIndex = 0;
    QString version = m_versions[m_currentVersionIndex];
    emit fetchProgress(QString("Checking Kubuntu version %1...").arg(version));

    QUrl releaseUrl(m_kubuntuBaseUrl + version + "/release/");
    QNetworkReply *releaseReply = fetchUrl(releaseUrl);
    if (releaseReply) {
        connect(releaseReply, &QNetworkReply::finished, this, &ReleaseFetch::onKubuntuReleasePageFinished);
    } else {
        emit fetchFailed("Failed to start Kubuntu release page request");
    }
}

void ReleaseFetch::onKubuntuReleasePageFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (m_canceled) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QString html = QString::fromUtf8(reply->readAll());
        QString version = m_versions[m_currentVersionIndex];
        QString filename = QString("kubuntu-%1-desktop-amd64.iso").arg(version);

        // Check if ISO exists
        if (html.contains(filename)) {
            IsoRelease release;
            release.name = QString("Kubuntu %1").arg(version);
            release.version = version;
            release.filename = filename;
            release.url = m_kubuntuBaseUrl + version + "/release/" + filename;
            release.distro = "Kubuntu";

            // Fetch SHA256SUMS
            QUrl sha256Url(m_kubuntuBaseUrl + version + "/release/SHA256SUMS");
            QNetworkReply *sha256Reply = fetchUrl(sha256Url);
            if (sha256Reply) {
                // Store the release temporarily and fetch SHA256
                m_kubuntuReleases.append(release);
                connect(sha256Reply, &QNetworkReply::finished, this, &ReleaseFetch::onKubuntuSha256Finished);
                return; // Don't proceed to next version yet
            } else {
                // Add without SHA256 if fetch fails
                m_kubuntuReleases.append(release);
            }
        }
    }

    // Move to next version
    m_currentVersionIndex++;
    if (m_currentVersionIndex < m_versions.size()) {
        QString version = m_versions[m_currentVersionIndex];
        emit fetchProgress(QString("Checking Kubuntu version %1...").arg(version));

        QUrl releaseUrl(m_kubuntuBaseUrl + version + "/release/");
        QNetworkReply *releaseReply = fetchUrl(releaseUrl);
        if (releaseReply) {
            connect(releaseReply, &QNetworkReply::finished, this, &ReleaseFetch::onKubuntuReleasePageFinished);
        }
    } else {
        // All Kubuntu versions processed
        m_kubuntuComplete = true;
        emit fetchProgress("Kubuntu fetch complete");
        checkIfComplete();
    }
}

void ReleaseFetch::onKubuntuSha256Finished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (m_canceled) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError && !m_kubuntuReleases.isEmpty()) {
        QString sha256Content = QString::fromUtf8(reply->readAll());
        IsoRelease &release = m_kubuntuReleases.last();
        release.sha256 = extractSHA256(sha256Content, release.filename);
    }

    // Continue with next version
    m_currentVersionIndex++;
    if (m_currentVersionIndex < m_versions.size()) {
        QString version = m_versions[m_currentVersionIndex];
        emit fetchProgress(QString("Checking Kubuntu version %1...").arg(version));

        QUrl releaseUrl(m_kubuntuBaseUrl + version + "/release/");
        QNetworkReply *releaseReply = fetchUrl(releaseUrl);
        if (releaseReply) {
            connect(releaseReply, &QNetworkReply::finished, this, &ReleaseFetch::onKubuntuReleasePageFinished);
        }
    } else {
        // All Kubuntu versions processed
        m_kubuntuComplete = true;
        emit fetchProgress("Kubuntu fetch complete");
        checkIfComplete();
    }
}

void ReleaseFetch::onFedoraReleasesFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply)
        return;

    reply->deleteLater();

    if (m_canceled) {
        return;
    }

    if (reply->error() != QNetworkReply::NoError) {
        emit fetchFailed("Failed to fetch Fedora releases: " + reply->errorString());
        return;
    }

    QByteArray data = reply->readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        emit fetchFailed("Failed to parse Fedora JSON: " + error.errorString());
        return;
    }

    if (!doc.isArray()) {
        emit fetchFailed("Invalid Fedora JSON format");
        return;
    }

    QJsonArray releases = doc.array();
    emit fetchProgress("Processing Fedora releases...");

    // Filter for KDE/Plasma variants
    for (const QJsonValue &value : releases) {
        QJsonObject releaseObj = value.toObject();
        QString variant = releaseObj["variant"].toString();
        QString subvariant = releaseObj["subvariant"].toString();

        // Look for KDE/Plasma variants
        if (variant.contains("KDE", Qt::CaseInsensitive) || subvariant.contains("KDE", Qt::CaseInsensitive) || variant.contains("Plasma", Qt::CaseInsensitive)
            || subvariant.contains("Plasma", Qt::CaseInsensitive)) {
            IsoRelease release;
            release.name = QString("Fedora %1 %2").arg(releaseObj["version"].toString(), variant);
            release.version = releaseObj["version"].toString();
            release.url = releaseObj["link"].toString();
            release.sha256 = releaseObj["sha256"].toString();
            release.distro = "Fedora";

            // Extract filename from URL
            QUrl url(release.url);
            release.filename = url.fileName();

            m_fedoraReleases.append(release);
        }
    }

    m_fedoraComplete = true;
    emit fetchProgress("Fedora fetch complete");
    checkIfComplete();
}

void ReleaseFetch::fetchKdeNeonReleases()
{
    emit fetchProgress("Adding KDE neon release...");

    // Create KDE neon release directly since we know the structure
    IsoRelease release;
    release.name = "KDE neon User Edition";
    release.version = "current";
    release.filename = "neon-user-current.iso";
    release.url = QString(KDENEON_URL) + release.filename;
    release.distro = "KDE neon";
    // SHA256 URL is known but we'll let the download process handle verification
    release.sha256 = QString(KDENEON_URL) + "neon-user-current.sha256sum";

    m_kdeNeonReleases.append(release);
    m_kdeNeonComplete = true;
    emit fetchProgress("KDE neon release added");
    checkIfComplete();
}

QStringList ReleaseFetch::parseVersions(const QString &html)
{
    QStringList versions;
    QRegularExpression versionRegex(R"(<a href="([0-9]+\.[0-9]+(?:\.[0-9]+)?)/")");
    QRegularExpressionMatchIterator iter = versionRegex.globalMatch(html);

    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString version = match.captured(1);
        versions.append(version);
    }

    return versions;
}

QString ReleaseFetch::extractSHA256(const QString &sha256Content, const QString &filename)
{
    QRegularExpression sha256Regex(QString("([a-f0-9]{64})\\s+\\*?%1").arg(QRegularExpression::escape(filename)));
    QRegularExpressionMatch match = sha256Regex.match(sha256Content);

    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}

void ReleaseFetch::checkIfComplete()
{
    if (m_kubuntuComplete && m_fedoraComplete && m_kdeNeonComplete) {
        // Stop the timeout timer since we completed successfully
        m_timeoutTimer.stop();

        emit fetchProgress("All fetches complete, selecting latest releases...");

        QList<IsoRelease> finalReleases;

        // Get latest Kubuntu
        IsoRelease latestKubuntu = getLatestKubuntu();
        if (!latestKubuntu.version.isEmpty()) {
            finalReleases.append(latestKubuntu);
        }

        // Get latest Fedora KDE
        IsoRelease latestFedora = getLatestFedoraKDE();
        if (!latestFedora.version.isEmpty()) {
            finalReleases.append(latestFedora);
        }

        // Get latest KDE neon
        IsoRelease latestKdeNeon = getLatestKdeNeon();
        if (!latestKdeNeon.version.isEmpty()) {
            finalReleases.append(latestKdeNeon);
        }

        QJsonArray result = releasesToJson(finalReleases);

        // Cache the result
        const QString cacheKey = "latest_releases";
        s_releaseCache.insert(cacheKey, new QJsonArray(result));

        emit releasesReady(result);
    }
}

IsoRelease ReleaseFetch::getLatestKubuntu()
{
    if (m_kubuntuReleases.isEmpty()) {
        return IsoRelease();
    }

    // Sort by version and return the latest
    std::sort(m_kubuntuReleases.begin(), m_kubuntuReleases.end(), [](const IsoRelease &a, const IsoRelease &b) {
        return a.version > b.version;
    });

    return m_kubuntuReleases.first();
}

IsoRelease ReleaseFetch::getLatestFedoraKDE()
{
    if (m_fedoraReleases.isEmpty()) {
        return IsoRelease();
    }

    // Sort by version and return the latest
    std::sort(m_fedoraReleases.begin(), m_fedoraReleases.end(), [](const IsoRelease &a, const IsoRelease &b) {
        return a.version.toInt() > b.version.toInt();
    });

    return m_fedoraReleases.first();
}

IsoRelease ReleaseFetch::getLatestKdeNeon()
{
    if (m_kdeNeonReleases.isEmpty()) {
        return IsoRelease();
    }

    // KDE neon only has one "current" release, so return the first (and only) one
    return m_kdeNeonReleases.first();
}

QJsonArray ReleaseFetch::releasesToJson(const QList<IsoRelease> &releases)
{
    QJsonArray jsonArray;

    for (const auto &release : releases) {
        QJsonObject releaseObj;
        releaseObj["name"] = release.name;
        releaseObj["version"] = release.version;
        releaseObj["url"] = release.url;
        releaseObj["sha256"] = release.sha256;
        releaseObj["filename"] = release.filename;
        releaseObj["distro"] = release.distro;
        jsonArray.append(releaseObj);
    }

    return jsonArray;
}

void ReleaseFetch::onTimeout()
{
    if (m_canceled) {
        return;
    }

    emit fetchProgress("Network request timed out after 10 seconds");

    // Cancel all ongoing operations
    m_canceled = true;

    // Return empty list as requested
    QJsonArray emptyResult;
    emit releasesReady(emptyResult);
}

#include "moc_releasefetch.cpp"
