/*
    SPDX-FileCopyrightText: 2019 Farid Boudedja <farid.boudedja@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "isoverifier.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QSignalSpy>

#ifdef _USE_GPG
#include <QGpgME/Protocol>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/ImportJob>

#include <gpgme++/verificationresult.h>
#include <gpgme++/importresult.h>
#endif

#include <KLocalizedString>

IsoVerifier::IsoVerifier(const QString &filePath)
    : m_filePath(filePath),
      m_error(),
      m_verificationMean(VerificationMean::None)
{
    qRegisterMetaType<VerifyResult>();
}

void IsoVerifier::verifyIso()
{
    QFileInfo fileInfo(m_filePath);
    QString fileName = fileInfo.fileName();
    QString keyFingerprint;

    if (fileName.startsWith("neon-")
        && importSigningKey("neon-signing-key.gpg", keyFingerprint)) {
        m_verificationMean = VerificationMean::DotSigFile;
    } else if (fileName.startsWith("archlinux-")
               && importSigningKey("arch-signing-key.gpg", keyFingerprint)) {
        m_verificationMean = VerificationMean::DotSigFile;
    } else if (fileName.startsWith("kubuntu-")
               && importSigningKey("ubuntu-signing-key.gpg", keyFingerprint)) {
        m_verificationMean = VerificationMean::Sha256SumsFile;
    } else if (fileName.startsWith("ubuntu-")
               && importSigningKey("ubuntu-signing-key.gpg", keyFingerprint)) {
        m_verificationMean = VerificationMean::Sha256SumsFile;
    } else if (fileName.startsWith("netrunner-")) {
        m_verificationMean = VerificationMean::Sha256SumInput;
    } else if (fileName.startsWith("debian-")) {
        m_verificationMean = VerificationMean::Sha256SumInput;
    } else {
        m_error = i18n("Could not verify as a known distro image.");
        m_isIsoValid = VerifyResult::KeyNotFound;
    }

    switch (m_verificationMean) {
    case VerificationMean::DotSigFile:
        verifyWithDotSigFile(keyFingerprint);
        break;
    case VerificationMean::Sha256SumsFile:
        verifyWithSha256SumsFile(keyFingerprint);
        break;
    case VerificationMean::Sha256SumInput:
        emit inputRequested(i18n("SHA256 Checksum"),
                            i18n("Paste the SHA256 checksum for this ISO:"));
        break;
    default:
        emit finished(m_isIsoValid, m_error);
        break;
    }
}

void IsoVerifier::verifyWithInputText(bool ok, const QString &text)
{
    switch (m_verificationMean) {
    case VerificationMean::Sha256SumInput:
        verifyWithSha256Sum(ok, text);
        break;
    default:
        emit finished(m_isIsoValid, m_error);
        break;
    }
}

bool IsoVerifier::importSigningKey(const QString &fileName, QString &keyFingerprint)
{
    QString signingKeyFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, fileName);
    if (signingKeyFile.isEmpty()) {
        qDebug() << "error can't find signing key" << signingKeyFile;
        return false;
    }

    QFile signingKey(signingKeyFile);
    if (!signingKey.open(QIODevice::ReadOnly)) {
        return false;
    }
    QByteArray signingKeyData = signingKey.readAll();

#ifdef _USE_GPG
    QGpgME::ImportJob *importJob = QGpgME::openpgp()->importJob();
    GpgME::ImportResult importResult = importJob->exec(signingKeyData);

    if (!(importResult.numConsidered() == 1
          && (importResult.numImported() == 1
              || importResult.numUnchanged() == 1))) {
        qDebug() << "Could not import gpg signature";
        return false;
    }

    keyFingerprint = QString(importResult.import(0).fingerprint());

    return true;
#endif
    return false;
}

void IsoVerifier::verifyWithDotSigFile(const QString &keyFingerprint)
{
    QString sigFilePath = m_filePath + ".sig";
    QFileInfo fileInfo(sigFilePath);
    QString sigFileName = fileInfo.fileName();
    if (!QFile::exists(sigFilePath)) {
        m_error = i18n("Could not find %1, please download PGP signature file "
                       "to same directory.", sigFileName);
        emit finished(m_isIsoValid, m_error); return;
    }

    auto signatureFile = std::shared_ptr<QIODevice>(new QFile(sigFilePath));
    if (!signatureFile->open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not open signature file");
        emit finished(m_isIsoValid, m_error); return;
    }

    auto isoFile = std::shared_ptr<QIODevice>(new QFile(m_filePath));
    if (!isoFile->open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not open ISO image");
        emit finished(m_isIsoValid, m_error); return;
    }


#ifdef _USE_GPG
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    connect(job, &QGpgME::VerifyDetachedJob::result, this, [this](GpgME::VerificationResult result)
    {
        GpgME::Signature signature = result.signature(0);
        this->summaryResult = signature.summary();
        Q_EMIT asyncDone();
    });
    job->start(signatureFile, isoFile);
    QSignalSpy spy(this, SIGNAL(asyncDone()));
    spy.wait(20000); // Set a long timeout as it can take time to read the whole ISO file

    // async api returns enums
    if (summaryResult == 0) {        // GpgME::Signature::None
        m_isIsoValid = VerifyResult::Successful;
    } else if (summaryResult == 1) { // GpgME::Signature::Valid
        m_isIsoValid = VerifyResult::Successful;
    }
    else if (summaryResult == 8) {   // GpgME::Signature::KeyRevoked
        m_error = i18n("Key is revoked.");
        m_isIsoValid = VerifyResult::Failed;
    } else {
        m_error = i18n("Uses wrong signature.");
        m_isIsoValid = VerifyResult::Failed;
    }
#else
        m_error = i18n("This app is built without verification support.");
        m_isIsoValid = VerifyResult::KeyNotFound;
#endif

    emit finished(m_isIsoValid, m_error);
}

void IsoVerifier::verifyWithSha256SumsFile(const QString &keyFingerprint)
{
    QFileInfo fileInfo(m_filePath);
    QFile checksumsFile(fileInfo.absolutePath() + "/SHA256SUMS");
    if (!checksumsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_error = i18n("Could not open SHA256SUMS file, please download to same directory");
        emit finished(m_isIsoValid, m_error); return;
    }

    // Extract checksum from the SHA256SUMS file
    QString checksum;
    QRegularExpression rx("([abcdef\\d]+).." + fileInfo.fileName());
    QByteArray checksumsData = checksumsFile.readAll();

    QRegularExpressionMatch match = rx.match(checksumsData);

    if (match.hasMatch()) {
        checksum = match.captured(1);
    } else {
        m_error = i18n("Could not find checksum in SHA256SUMS file");
        emit finished(m_isIsoValid, m_error); return;
    }

    // Calculate SHA256 checksum of the ISO image
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QFile isoFile(m_filePath);
    if (!isoFile.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not read ISO image");
        emit finished(m_isIsoValid, m_error); return;
    }
    if (!hash.addData(&isoFile)) {
        m_error = i18n("Could not perform checksum");
        emit finished(m_isIsoValid, m_error); return;
    }
    QByteArray hashResult = hash.result();
    if (checksum != hashResult.toHex()) {
        m_error = i18n("Checksum of .iso file does not match value in SHA256SUMS file");
        emit finished(m_isIsoValid, m_error); return;
    }

    // Check GPG signature
    QString isoFileName = fileInfo.fileName();
    QFile signatureFile(fileInfo.absolutePath() + "/SHA256SUMS.gpg");
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not find SHA256SUMS.gpg, please download PGP signature file to same directory.");
        emit finished(m_isIsoValid, m_error); return;
    }


#ifdef _USE_GPG
    QByteArray signatureData = signatureFile.readAll();
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    GpgME::VerificationResult result = job->exec(signatureData, checksumsData);
    GpgME::Signature signature = result.signature(0);

    if (signature.summary() == GpgME::Signature::None
        && signature.fingerprint() == keyFingerprint) {
        m_isIsoValid = VerifyResult::Successful;
    } else if (signature.summary() & GpgME::Signature::Valid) {
        m_isIsoValid = VerifyResult::Successful;
    } else if (signature.summary() & GpgME::Signature::KeyRevoked) {
        m_error = i18n("Key is revoked.");
        m_isIsoValid = VerifyResult::Failed;
    } else {
        m_error = i18n("Uses wrong signature.");
        m_isIsoValid = VerifyResult::Failed;
    }
#else
        m_error = i18n("This app is built without verification support.");
        m_isIsoValid = VerifyResult::KeyNotFound;
#endif
    emit finished(m_isIsoValid, m_error);
}

void IsoVerifier::verifyWithSha256Sum(bool ok, const QString &checksum)
{
    if (ok && !checksum.isEmpty()) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        QFile iso(m_filePath);
        if (!iso.open(QIODevice::ReadOnly)) {
            m_error = i18n("Could not read ISO image");
            goto finish;
        }
        if (!hash.addData(&iso)) {
            m_error = i18n("Could not perform checksum");
            goto finish;
        }
        QByteArray hashResult = hash.result();

        if (checksum.toLower() == hashResult.toHex().toLower()) {
            m_isIsoValid = VerifyResult::Successful;
            goto finish;
        } else {
            m_error = i18n("Checksum did not match");
            m_isIsoValid = VerifyResult::Failed;
            goto finish;
        }
    }

    m_error = i18n("Requires an SHA256 checksum");

finish:
    emit finished(m_isIsoValid, m_error);
}

#include "moc_isoverifier.cpp"
