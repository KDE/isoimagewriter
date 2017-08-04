/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright 2017  KDE neon <email>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QFile>
#include <QByteArray>
#include <QDebug>
#include <QFileInfo>
#include <QCryptographicHash>

#include <QGpgME/Protocol>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/ImportJob>
#include <gpgme++/verificationresult.h>
#include <gpgme++/importresult.h>

#include "verifyisoworker.h"

#include <KLocalizedString>

VerifyISOWorker::VerifyISOWorker(QString filename, Distro distro) {
    m_resultReady = false;
    m_filename = filename;
    m_distro = distro;
}

void VerifyISOWorker::run() {
    qDebug() << "run()";
    if (m_distro == Neon || m_distro == Arch) {
        doVerificationNeon();
    } else {
        doVerificationKubuntu();
    }
}

void VerifyISOWorker::doVerificationNeon() {
    qDebug() << "doVerification()";
    m_result = true;
    QFile signatureFile(m_filename + ".sig");
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",signatureFile.errorString();
    }
    QByteArray signatureData = signatureFile.readAll();
    QFile isoFile(m_filename);
    if (!isoFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",isoFile.errorString();
    }
    QByteArray isoData = isoFile.readAll();
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    qDebug() << "doVerification() about to exec";
    GpgME::VerificationResult result = job->exec(signatureData, isoData);
    qDebug() << "doVerification() execed";
    qDebug() << "numSignatures " << result.numSignatures();
    qDebug() << "filename " << result.fileName();
    GpgME::Signature signature = result.signature(0);
    qDebug() << "fingerprint " << signature.fingerprint();
    if (m_distro == Neon && (strcmp(signature.fingerprint(), "348C8651206633FD983A8FC4DEACEA00075E1D76") == 0 ||
        strcmp(signature.fingerprint(), "DEACEA00075E1D76") == 0)) {
        qDebug() << "Uses right Neon signature!";
    } else if (m_distro == Arch && strcmp(signature.fingerprint(), "4AA4767BBC9C4B1D18AE28B77F2D434B9741E8AC") == 0) {
        qDebug() << "Uses right Arch signature!";
    } else {
        qDebug() << "Uses wrong signature!!";
        m_error = i18n("Uses wrong signature.");
        m_result = false;
    }
    if (signature.summary() & GpgME::Signature::KeyRevoked) {
        qDebug() << "Key is revoked" << signature.summary();
        m_error = i18n("Key is revoked.");
        m_result = false;
    }
    m_resultReady = true;
}

void VerifyISOWorker::doVerificationKubuntu() {
    m_result = true;
    QFileInfo fi(m_filename);
    QString fileNameChecksums = fi.absolutePath() + "/SHA256SUMS";
    QString fileNameChecksumsSig = fi.absolutePath() + "/SHA256SUMS.gpg";
    QFile fileChecksums(fileNameChecksums);
    if (!fileChecksums.open(QIODevice::ReadOnly | QIODevice::Text)) {
        m_error = i18n("Could not open SHA256SUMS file, please download to same directory");
        m_result = false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QFile iso(m_filename);
    if (!iso.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not read file");
    }
    // slow, threadify me
    if (!hash.addData(&iso)) {
        m_error = i18n("Could not perform checksum");
        m_result = false;
    }
    QByteArray hashResult = hash.result();
    
    QString checksum;
    qDebug() << "regex " << "([\\dabcdef]+) \*"+fi.fileName();
    //QRegExp rx("([\\dabcdef]+) \*"+fi.fileName());
    QRegExp rx("([abcdef\\d]+).."+fi.fileName());
    QByteArray checksumData = fileChecksums.readAll();
    qDebug() << "checksumData " << checksumData;
    int pos = rx.indexIn(QString(checksumData));
    if (pos > -1) {
        checksum = rx.cap(1);
    } else {
        m_error = i18n("Could not find checksum in SHA256SUMS file");
        m_result = false;
    }
    qDebug() << "checksum: " << checksum;
    if (checksum != hashResult.toHex()) {
        m_error = i18n("Checksum of .iso file does not match value in SHA256SUMS file");
        m_result = false;
    }

    // check gpg signature
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!QFile::exists(fileNameChecksumsSig)) {
        qDebug() << "does not exist SHA256SUMS.gpg" << "SHA256SUMS.gpg";
        m_error = i18n("Could not find SHA256SUMS.gpg, please download PGP signature file to same directory.");
        m_result = false;
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
    if (strcmp(signature.fingerprint(), "46181433FBB75451") == 0) {
        qDebug() << "Uses right signature!" << signature.fingerprint();
    } else {
        qDebug() << "Uses wrong signature!!" << signature.fingerprint();
        m_error = i18n("Uses wrong signature.");
        m_result = false;
    }
    if (signature.summary() & GpgME::Signature::KeyRevoked) {
        qDebug() << "Key is revoked" << signature.summary();
        m_error = i18n("Key is revoked.");
        m_result = false;
    }
    m_resultReady = true;
}

bool VerifyISOWorker::isResultReady() {
    return m_resultReady;
}

bool VerifyISOWorker::getResult() {
    return m_result;
}

QString VerifyISOWorker::getErrorMessage() {
    return m_error;
}
