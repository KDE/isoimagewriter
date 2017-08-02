/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright 2017 Jonathan Riddell <jr@jriddell.org>
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
#include <QFile>
#include <QFileInfo>
#include <QDebug>
#include <QStandardPaths>

#include <KLocalizedString>

#include "verifyiso.h"

#include <QGpgME/Protocol>
#include <QGpgME/VerifyDetachedJob>
#include <QGpgME/ImportJob>
#include <gpgme++/verificationresult.h>
#include <gpgme++/importresult.h>

VerifyISO::VerifyISO(QString filename): m_filename(filename)
{
}

bool VerifyISO::verifyFileExists() {
    if (!QFile::exists(getFilename())) {
        m_error = i18n("ISO File %1 does not exist", getFilename());
        return false;
    } 
    return true;
}

bool VerifyISO::verifySignatureFileExists(QString filename) {
    qDebug() << "verifySignatureFileExists" << filename;
    QFileInfo fileInfo(filename);
    QString sigFileName = fileInfo.fileName();
    qDebug() << "verifySignatureFileExists sigFileName" << sigFileName;
    if (!QFile::exists(filename)) {
        m_error = i18n("Could not find %1, please download PGP signature file to same directory.", sigFileName);
        return false;
    }
    return true;
}

bool VerifyISO::verifyFileMatches(QString startsWith) {
    QFileInfo fileInfo(getFilename());
    QString fileName = fileInfo.fileName();
    if (!fileName.startsWith(startsWith)) {
        m_error = i18n("Filename does not match %1 ISO files", m_humanReadableDistroName);
        return false;
    }
    return true;
}

bool VerifyISO::importSigningKey(QString keyFilename) {
    QString signingKeyFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, keyFilename);
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
    qDebug() << "numConsidered " << importResult.numConsidered();
    qDebug() << "numImported " << importResult.numImported();
    qDebug() << "numUnchanged " << importResult.numUnchanged();
    if (!(importResult.numConsidered() == 1 && (importResult.numImported() == 1 || importResult.numUnchanged() == 1))) {
        qDebug() << "Could not import gpg signature";
        return false;
    }
    return true;
}

QString VerifyISO::getFilename() const
{
    return m_filename;
}

QString VerifyISO::getError() const
{
    return m_error;
}

void VerifyISO::setFilename(const QString& filename)
{
    if (m_filename == filename) {
        return;
    }

    m_filename = filename;
}
