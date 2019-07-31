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

signals:
    void finished(const bool &isIsoValid, const QString &error);

private:
    QString m_filePath;
    QString m_error;
    bool m_isIsoValid;
    enum VerificationMean { DotSigFile, Sha256SumsFile } m_verificationMean;

    bool importSigningKey(const QString &fileName, QString &keyFingerPrint);
    void verifyWithDotSigFile(const QString &keyFingerPrint);
    void verifyWithSha256SumsFile();
};

#endif // ISOVERIFIER_H
