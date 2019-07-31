#include "isoverifier.h"

#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
#include <QStandardPaths>

#include <QGpgME/Protocol>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/ImportJob>
#include <gpgme++/verificationresult.h>
#include <gpgme++/importresult.h>

#include <KLocalizedString>

IsoVerifier::IsoVerifier(const QString &filePath)
    : m_filePath(filePath),
      m_error(),
      m_isIsoValid(false)
{}

void IsoVerifier::verifyIso()
{
    QFileInfo fileInfo(m_filePath);
    QString fileName = fileInfo.fileName();
    QString keyFingerprint;

    if (fileName.startsWith("neon-")) {
        m_verificationMean = VerificationMean::DotSigFile;
        if (!importSigningKey("neon-signing-key.gpg", keyFingerprint)) return;
    } else if (fileName.startsWith("archlinux-")) {
        m_verificationMean = VerificationMean::DotSigFile;
        if (!importSigningKey("arch-signing-key.gpg", keyFingerprint)) return;
    } else if (fileName.startsWith("kubuntu-")) {
        m_verificationMean = VerificationMean::Sha256SumsFile;
        if (!importSigningKey("ubuntu-signing-key.gpg", keyFingerprint)) return;
    } else {
        m_error = QString(i18n("Could not verify as a known distro image."));
        return;
    }

    switch (m_verificationMean) {
    case VerificationMean::DotSigFile:
        verifyWithDotSigFile(keyFingerprint);
        break;
    case VerificationMean::Sha256SumsFile:
        verifyWithSha256SumsFile();
        break;
    }

    emit finished(m_isIsoValid, m_error);
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
        qDebug() << "error" << signingKey.errorString();
        return false;
    }

    QByteArray signingKeyData = signingKey.readAll();
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
}

void IsoVerifier::verifyWithDotSigFile(const QString &keyFingerprint)
{
    QString sigFilePath = m_filePath + ".sig";
    QFileInfo fileInfo(sigFilePath);
    QString sigFileName = fileInfo.fileName();
    if (!QFile::exists(sigFilePath)) {
        m_error = i18n("Could not find %1, please download PGP signature file "
                       "to same directory.", sigFileName);
        return;
    }

    QFile signatureFile(sigFilePath);
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not open signature file");
        return;
    }
    QByteArray signatureData = signatureFile.readAll();

    QFile isoFile(m_filePath);
    if (!isoFile.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not open ISO image");
        return;
    }
    QByteArray isoData = isoFile.readAll();

    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    GpgME::VerificationResult result = job->exec(signatureData, isoData);
    GpgME::Signature signature = result.signature(0);

    if (signature.summary() == GpgME::Signature::None
        && signature.fingerprint() == keyFingerprint) {
        m_isIsoValid = true;
    } else if (signature.summary() & GpgME::Signature::Valid) {
        m_isIsoValid = true;
    } else if (signature.summary() & GpgME::Signature::KeyRevoked) {
        m_error = i18n("Key is revoked.");
    } else {
        m_error = i18n("Uses wrong signature.");
    }
}

void IsoVerifier::verifyWithSha256SumsFile()
{
}
