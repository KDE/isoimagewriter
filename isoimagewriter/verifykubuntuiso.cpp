/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  KDE neon <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QGpgME/Protocol>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/ImportJob>
#include <gpgme++/verificationresult.h>
#include <gpgme++/importresult.h>

#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QInputDialog>
#include <QFileInfo>

#include <KLocalizedString>

#include "verifykubuntuiso.h"

VerifyKubuntuISO::VerifyKubuntuISO(QString filename) : VerifyISO(filename)
{
}

bool VerifyKubuntuISO::canVerify() {
    QStringList splits = m_filename.split('/');
    QFileInfo fi(m_filename);
    QString fileName = fi.fileName();
    if (!fileName.startsWith("kubuntu-")) {
        m_error = i18n("Filename does not match Netrunner ISO files");
        return false;
    }

    QString signingKeyFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "ubuntu-signing-key.gpg");
    if (signingKeyFile.isEmpty()) {
        qDebug() << "error can't find ubuntu-signing-key" << signingKeyFile;
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
    qDebug() << "numConsidered " << importResult.numConsidered();
    qDebug() << "numImported " << importResult.numImported();
    qDebug() << "numUnchanged " << importResult.numUnchanged();
    if (!(importResult.numConsidered() == 1 && (importResult.numImported() == 1 || importResult.numUnchanged() == 1))) {
        qDebug() << "Could not import gpg signature";
        return false;
    }
    return true;
}

bool VerifyKubuntuISO::isValid() {
    if (!verifyFilename()) {
        return false;
    }
    qDebug() << "m_filename " << m_filename;
    QFileInfo fi(m_filename);
    QString fileNameChecksums = fi.absolutePath() + "/SHA256SUMS";
    QString fileNameChecksumsSig = fi.absolutePath() + "/SHA256SUMS.gpg";
    qDebug() << "fileNameChecksums " << fileNameChecksums;
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QFile iso(m_filename);
    if (!iso.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not read file");
    }
    if (!hash.addData(&iso)) {
        m_error = i18n("Could not perform checksum");
        return false;
    }
    // slow, threadify me
    QByteArray hashResult = hash.result();
    
    QFile fileChecksums(fileNameChecksums);
    if (!fileChecksums.open(QIODevice::ReadOnly | QIODevice::Text)) {
            m_error = i18n("Could not open SHA256SUMS file, please download to same directory");
            return false;
    }
    // make regex for sha line
    // find sha line and sha
    // if sha matches good
    // if gpg matches good

        /*
        qDebug() << "text " << text;
        if (text == hashResult.toHex()) {
            return true;
        } else {
            m_error = i18n("Checksum did not match");
            return false;
        }
    }
    */

    // check gpg signature
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!QFile::exists(fileNameChecksumsSig)) {
        qDebug() << "does not exist SHA256SUMS.gpg" << "SHA256SUMS.gpg";
        m_error = i18n("Could not find SHA256SUMS.gpg, please download PGP signature file to same directory.");
        return false;
    }
    QFile signatureFile(fileNameChecksumsSig);
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",signatureFile.errorString();
    }
    QByteArray signatureData = signatureFile.readAll();
    QByteArray fileChecksumsData = fileChecksums.readAll();
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    GpgME::VerificationResult result = job->exec(signatureData, fileChecksumsData);
    qDebug() << "numSignatures " << result.numSignatures();
    qDebug() << "filename " << result.fileName();
    GpgME::Signature signature = result.signature(0);
    qDebug() << "fingerprint " << signature.fingerprint();
    if (strcmp(signature.fingerprint(), "C5986B4F1257FFA86632CBA746181433FBB75451") == 0) {
        qDebug() << "Uses right signature!" << signature.fingerprint();
    } else {
        qDebug() << "Uses wrong signature!!" << signature.fingerprint();
        m_error = i18n("Uses wrong signature.");
        return false;
    }
    if (signature.summary() & GpgME::Signature::KeyRevoked) {
        qDebug() << "Key is revoked" << signature.summary();
        m_error = i18n("Key is revoked.");
        return false;
    }

    return true;
}
