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

#include <QDebug>
#include <QFile>
#include <QStandardPaths>
#include <QCryptographicHash>

#include <KLocalizedString>

#include "verifynetrunneriso.h"

VerifyNetrunnerISO::VerifyNetrunnerISO(QString filename) : VerifyISO(filename)
{
}

bool VerifyNetrunnerISO::canVerify() {
    QStringList splits = m_filename.split('/');
    QString fileName = splits[splits.size()-1];
    if (!fileName.startsWith("netrunner-")) {
        m_error = i18n("Filename does not match Netrunner ISO files");
        return false;
    }
    return true;
}

bool VerifyNetrunnerISO::isValid() {
    if (!verifyFilename()) {
        return false;
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    QFile iso(m_filename);
    if (!iso.open(QIODevice::ReadOnly)) {
        m_error = i18n("Could not read file");
    }
    hash.addData(&iso);
    QByteArray hashResult = hash.result();
    qDebug() << "result " << hashResult;
    return true;
}
