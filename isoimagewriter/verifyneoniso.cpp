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
    if (!importSigningKey("neon-signing-key.gpg")) {
        return false;
    }
    return true;
}

bool VerifyNeonISO::isValid() {
    if (!verifyFileExists()) {
        return false;
    }
    if (!verifySignatureFileExists(m_filename+".sig")) {
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
    QByteArray isoData = isoFile.readAll();
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    GpgME::VerificationResult result = job->exec(signatureData, isoData);
    qDebug() << "numSignatures " << result.numSignatures();
    qDebug() << "filename " << result.fileName();
    GpgME::Signature signature = result.signature(0);
    qDebug() << "fingerprint " << signature.fingerprint();
    if (strcmp(signature.fingerprint(), "348C8651206633FD983A8FC4DEACEA00075E1D76") == 0) {
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
