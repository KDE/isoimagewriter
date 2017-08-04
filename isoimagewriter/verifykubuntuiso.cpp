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
#include <QCryptographicHash>
#include <QInputDialog>
#include <QFileInfo>
#include <QCoreApplication>

#include <KLocalizedString>

#include "verifykubuntuiso.h"
#include "verifyisoworker.h"

VerifyKubuntuISO::VerifyKubuntuISO(QString filename) : VerifyISO(filename)
{
    m_humanReadableDistroName = "Kubuntu";
}

bool VerifyKubuntuISO::canVerify() {
    if (!verifyFileMatches("kubuntu-")) {
        return false;
    }
    if (!importSigningKey("ubuntu-signing-key.gpg")) {
        return false;
    }
    return true;
}

bool VerifyKubuntuISO::isValid() {
    if (!verifyFileExists()) {
        return false;
    }
    qDebug() << "m_filename " << m_filename;
    QFileInfo fi(m_filename);
    QString fileNameChecksums = fi.absolutePath() + "/SHA256SUMS";
    QString fileNameChecksumsSig = fi.absolutePath() + "/SHA256SUMS.gpg";
    if (!verifySignatureFileExists(fileNameChecksums)) {
        return false;
    }
    if (!verifySignatureFileExists(fileNameChecksumsSig)) {
        return false;
    }

    VerifyISOWorker* verifyISOWorker = new VerifyISOWorker(m_filename, Kubuntu);
    connect(verifyISOWorker, &QThread::finished, verifyISOWorker, &QObject::deleteLater);
    verifyISOWorker->start();
    while (verifyISOWorker->isResultReady() == false) {
        QCoreApplication::processEvents();
    }
    if (verifyISOWorker->getResult() == false) {
        m_error = verifyISOWorker->getErrorMessage();
        return false;
    }

    return true;
}
