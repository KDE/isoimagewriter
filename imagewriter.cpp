#include <QThread>

#include "imagewriter.h"

ImageWriter::ImageWriter(QObject *parent) :
    QObject(parent),
    m_CancelWriting(false)
{
}

void ImageWriter::writeImage()
{
/*
    Pseudo-code implementation
    ==========================
    external variables: imagename, devicedata;

    fi = openfile(imagename);
    foreach (vol in devicedata->volumes) unmountvolume(vol);
    fd = openfile(devicedata->physicalname);
    while (readblock(fd, buf, 1MB) > 0) {
        if (writeblock(fi, buf) != SUCCESS) {
            messagebox("Sh** happened!");
            return;
        }
        emit blockWritten(1);
        if (m_CancelWriting)
            break;
    }
    close(fd);
    close(fi);
 */
    // DBG: Test implementation
    for (int i = 0; i < 20; ++i)
    {
        thread()->msleep(1000);
        m_Mutex.lock();
        if (m_CancelWriting)
        {
            m_Mutex.unlock();
            break;
        }
        emit blockWritten(1);
        m_Mutex.unlock();
    }
    emit finished();
}

void ImageWriter::cancelWriting()
{
    m_Mutex.lock();
    m_CancelWriting = true;
    m_Mutex.unlock();
}
