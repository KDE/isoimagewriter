/*
    SPDX-FileCopyrightText: 2019 Farid Boudedja <farid.boudedja@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ISOVERIFIER_H
#define ISOVERIFIER_H

#include <QObject>

class IsoVerifier : public QObject
{
    Q_OBJECT

public:
    explicit IsoVerifier(const QString &filePath);

    enum class VerifyResult {
        Successful,
        Failed,
        KeyNotFound,
    };
    Q_ENUM(VerifyResult);

public slots:
    void verifyIso();
    void verifyWithInputText(bool ok, const QString &text);

signals:
    void finished(IsoVerifier::VerifyResult result, const QString &error);
    void inputRequested(const QString &title, const QString &body);

private:
    QString m_filePath;
    QString m_error;
    VerifyResult m_isIsoValid = VerifyResult::Failed;
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
