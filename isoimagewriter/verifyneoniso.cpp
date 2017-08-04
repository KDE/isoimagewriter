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
#include <QThread>
#include <QCoreApplication>

#include <KLocalizedString>

#include "verifyneoniso.h"
#include "verifyisoworker.h"

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
    if (!verifySignatureFileExists(m_filename + ".sig")) {
        return false;
    }
    QFile signatureFile(m_filename + ".sig");
    if (!signatureFile.open(QIODevice::ReadOnly)) {
        qDebug() << "error",signatureFile.errorString();
    }
    VerifyISOWorker* verifyISOWorker = new VerifyISOWorker(m_filename, Neon);
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
