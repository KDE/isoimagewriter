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

#include <QDebug>
#include <QFile>

#include <KLocalizedString>

#include "verifyneoniso.h"

VerifyNeonISO::VerifyNeonISO(QString filename) : VerifyISO(filename)
{
}

bool VerifyNeonISO::canVerify() {
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!fileName.startsWith("neon-")) {
        m_error = i18n("Filename does not match KDE neon ISO files");
        return false;
    }
    return true;
}

bool VerifyNeonISO::isValid() {
    if (!verifyFilename()) {
        return false;
    }
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!QFile::exists(m_filename+".sig")) {
        qDebug() << "does not exist .sig" << fileName+".sig";
        m_error = i18n("Could not find %1, please download PGP signature file to same directory.", fileName+".sig");
        return false;
    }
    QGpgME::VerifyDetachedJob *job = QGpgME::openpgp()->verifyDetachedJob();
    //GpgME::VerificationResult* result = job->exec(QStringList() << QStringLiteral("neon@kde.org"), m_filename);
    return true;
}
