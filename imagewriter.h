#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <QObject>
#include <QMutex>

#include "maindialog.h"

class ImageWriter : public QObject
{
    Q_OBJECT

public:
    explicit ImageWriter(const QString& ImageFile, UsbDevice* Device, QObject *parent = 0);

protected:
    bool m_CancelWriting;
    QMutex m_Mutex;
    UsbDevice* m_Device;
    QString m_ImageFile;

signals:
    void finished();
    void blockWritten(int count);
    void success();
    void error(QString msg);
    void cancelled();

public slots:
    void writeImage();
    void cancelWriting();
};

#endif // IMAGEWRITER_H
