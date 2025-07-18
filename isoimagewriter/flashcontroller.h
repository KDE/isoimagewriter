
#ifndef FLASHCONTROLLER_H
#define FLASHCONTROLLER_H

#include <QObject>
#include <QQmlEngine>
#include "imagewriter.h"
#include "usbdevice.h"

class FlashController : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(bool isWriting READ isWriting NOTIFY isWritingChanged)
    Q_PROPERTY(double progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit FlashController(QObject *parent = nullptr);
    ~FlashController();

    bool isWriting() const { return m_isWriting; }
    double progress() const { return m_progress; }
    QString statusMessage() const { return m_statusMessage; }
    QString errorMessage() const { return m_errorMessage; }

public slots:
    void startFlashing(const QString& isoPath, UsbDevice* device);
    void cancelFlashing();

signals:
    void isWritingChanged();
    void progressChanged();
    void statusMessageChanged();
    void errorMessageChanged();
    void flashCompleted();
    void flashFailed(const QString& error);

private slots:
    void onWriterProgress(int percent);
    void onWriterSuccess(const QString& msg);
    void onWriterError(const QString& msg);
    void onWriterCancelled();
    void onWriterFinished();

private:
    ImageWriter* m_writer;
    bool m_isWriting;
    double m_progress;
    QString m_statusMessage;
    QString m_errorMessage;

    void setIsWriting(bool writing);
    void setProgress(double progress);
    void setStatusMessage(const QString& message);
    void setErrorMessage(const QString& message);
};

#endif // FLASHCONTROLLER_H