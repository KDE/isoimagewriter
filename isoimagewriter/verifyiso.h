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

#ifndef VERIFYISO_H
#define VERIFYISO_H

#include <QObject>

/**
 * @todo write docs
 */
class VerifyISO : public QObject
{
    Q_OBJECT
    
public:
    /**
     * Default constructor
     */
    VerifyISO(QString filename);

    /**
     * @return the m_fileName
     */
    QString getFilename() const;
    
    /**
     * @return the m_error string
     */
    QString getError() const;
    bool verifyFileExists();
    bool verifyFileMatches(QString startsWith);
    bool importSigningKey(QString keyFilename);
    bool verifySignatureFileExists(QString filename);

    virtual bool canVerify() = 0;
    virtual bool isValid() = 0;
    QString m_error;

public Q_SLOTS:
    /**
     * Sets the m_fileName.
     *
     * @param m_filename the new m_fileName
     */
    void setFilename(const QString& filename);

protected:
    QString m_filename;
    QString m_humanReadableDistroName;
};

#endif // VERIFYISO_H
