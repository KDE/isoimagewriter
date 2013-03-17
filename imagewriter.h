#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

////////////////////////////////////////////////////////////////////////////////
// ImageWriter is a class for writing image file to the USB flash disk


#include <QObject>
#include <QMutex>

#include "maindialog.h"

class ImageWriter : public QObject
{
    Q_OBJECT

public:
    explicit ImageWriter(const QString& ImageFile, UsbDevice* Device, QObject *parent = 0);

protected:
    // Information about the selected USB flash disk
    UsbDevice* m_Device;
    // Source image file (full path)
    QString m_ImageFile;
    // Flag used for cancelling the operation by user
    bool m_CancelWriting;
    // Mutex for synchronizing access to m_CancelWriting member
    QMutex m_Mutex;

signals:
    // Emitted when writeImage is finished for any reason
    void finished();
    // Emitted on successful completion
    void success();
    // Emitted when something wrong happened, <msg> is the error message
    void error(QString msg);
    // Emitted when processed the cancel request from user
    void cancelled();
    // Emitted each time a block is written, <count> is a number of DEFAULT_UNIT-size blocks
    void blockWritten(int count);

public slots:
    // The main method that writes the image
    void writeImage();
    // Implements reaction to the cancel request from user
    void cancelWriting();
};

#endif // IMAGEWRITER_H
