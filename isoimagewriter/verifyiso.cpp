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
#include <QFile>

#include <KLocalizedString>

#include "verifyiso.h"

VerifyISO::VerifyISO(QString filename): m_filename(filename)
{
}

bool VerifyISO::verifyFilename() {
    if (!QFile::exists(getFilename())) {
        m_error = i18n("ISO File %1 does not exist");
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
