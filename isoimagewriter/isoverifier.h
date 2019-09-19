/*
 * Copyright 2019 Farid Boudedja <farid.boudedja@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ISOVERIFIER_H
#define ISOVERIFIER_H

#include <QObject>

class IsoVerifier : public QObject
{
    Q_OBJECT

public:
    explicit IsoVerifier(const QString &filePath);

public slots:
    void verifyIso();
    void verifyWithInputText(bool ok, const QString &text);

signals:
    void finished(const bool &isIsoValid, const QString &error);
    void inputRequested(const QString &title, const QString &body);

private:
    QString m_filePath;
    QString m_error;
    bool m_isIsoValid;
    enum VerificationMean {
        None,
        DotSigFile,
        Sha256SumsFile,
        Sha256SumInput
    } m_verificationMean;

    bool importSigningKey(const QString &fileName, QString &keyFingerPrint);
    void verifyWithDotSigFile(const QString &keyFingerPrint);
    void verifyWithSha256SumsFile(const QString &keyFingerPrint);
    void verifyWithSha256Sum(bool ok, const QString &checksum);
};

#endif // ISOVERIFIER_H
