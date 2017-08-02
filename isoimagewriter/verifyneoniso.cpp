/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017 Jonathan Riddell <jr@jriddell.org>
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

#include <KLocalizedString>

#include "verifyneoniso.h"

VerifyNeonISO::VerifyNeonISO(QString filename) : VerifyISO(filename)
{
    m_humanReadableDistroName = "KDE neon";
}

bool VerifyNeonISO::canVerify() {
    if (!verifyFileMatches("neon-")) {
        return false;
    }
    QString neonSigningKeyFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, "neon-signing-key.gpg");
    if (neonSigningKeyFile.isEmpty()) {
        qDebug() << "error can't find neon-signing-key" << neonSigningKeyFile;
        return false;
    }
    QFile signingKey(neonSigningKeyFile);
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

bool VerifyNeonISO::isValid() {
    if (!verifyFileExists()) {
        return false;
    }
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!QFile::exists(m_filename+".sig")) {
        qDebug() << "does not exist .sig" << fileName+".sig";
        m_error = i18n("Could not find %1, please download PGP signature file to same directory.", fileName+".sig");
        return false;
    }
    QFile signatureFile(m_filename + ".sig");
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",signatureFile.errorString();
    }
    QByteArray signatureData = signatureFile.readAll();
    QFile isoFile(m_filename);
    if (!isoFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",isoFile.errorString();
    }
    QByteArray isoData = signatureFile.readAll();
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    GpgME::VerificationResult result = job->exec(signatureData, isoData);
    qDebug() << "numSignatures " << result.numSignatures();
    qDebug() << "filename " << result.fileName();
    GpgME::Signature signature = result.signature(0);
    qDebug() << "fingerprint " << signature.fingerprint();
    if (strcmp(signature.fingerprint(), "DEACEA00075E1D76") == 0) {
        qDebug() << "Uses right signature!";
    } else {
        qDebug() << "Uses wrong signature!!";
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
