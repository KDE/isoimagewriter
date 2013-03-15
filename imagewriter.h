#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <QObject>
#include <QMutex>

class ImageWriter : public QObject
{
    Q_OBJECT

public:
    explicit ImageWriter(QObject *parent = 0);

protected:
    bool m_CancelWriting;
    QMutex m_Mutex;

signals:
    void finished();
    void blockWritten(int count);
    
public slots:
    void writeImage();
    void cancelWriting();
};

#endif // IMAGEWRITER_H
