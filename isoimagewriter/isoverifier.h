#ifndef ISOVERIFIER_H
#define ISOVERIFIER_H

#include <QObject>

class IsoVerifier : public QObject
{
    Q_OBJECT

public:
    IsoVerifier(const QString &filePath);

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
