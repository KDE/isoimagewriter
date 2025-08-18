/*
    SPDX-FileCopyrightText: 2019 Farid Boudedja <farid.boudedja@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef ISOVERIFIER_H
#define ISOVERIFIER_H

#include <QObject>
#include <QString>


class IsoVerifier : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString filePath READ filePath WRITE setFilePath NOTIFY filePathChanged)
    
public:
    explicit IsoVerifier(QObject *parent = nullptr);
    explicit IsoVerifier(const QString &filePath, QObject *parent = nullptr);
    
    // Property methods
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path);
    
    enum class VerifyResult {
        Successful,
        Failed,
        KeyNotFound,
        NoGpg,
    };
    Q_ENUM(VerifyResult)

public slots:
    void verifyIso();
    void verifyWithInputText(bool ok, const QString &text);
    void verifyWithSha256Sum(const QString &checksum);

signals:
    void finished(IsoVerifier::VerifyResult result, const QString &error);
    void inputRequested(const QString &title, const QString &body);
    void asyncDone();
    void filePathChanged();

private:

    int summaryResult;
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
