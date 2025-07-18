
#include "flashcontroller.h"
#include <QThread>
#include <KLocalizedString>
#include <iostream>
#include <QDebug>

FlashController::FlashController(QObject *parent)
    : QObject(parent)
    , m_writer(nullptr)
    , m_isWriting(false)
    , m_progress(0.0)
{
}

FlashController::~FlashController()
{
    if (m_writer) {
        m_writer->cancelWriting();
        m_writer->deleteLater();
    }
}

void FlashController::startFlashing(const QString& isoPath, UsbDevice* device)
{
    if (m_isWriting || !device) {
        return;
    }

    // Clean up any previous writer
    if (m_writer) {
        m_writer->deleteLater();
        m_writer = nullptr;
    }

    // Create new ImageWriter
    m_writer = new ImageWriter(isoPath, device, this);
    
    // Connect signals
    connect(m_writer, &ImageWriter::progressChanged, this, &FlashController::onWriterProgress);
    connect(m_writer, &ImageWriter::success, this, &FlashController::onWriterSuccess);
    connect(m_writer, &ImageWriter::error, this, &FlashController::onWriterError);
    connect(m_writer, &ImageWriter::cancelled, this, &FlashController::onWriterCancelled);
    connect(m_writer, &ImageWriter::finished, this, &FlashController::onWriterFinished);

    // Reset state
    setErrorMessage("");
    setProgress(0.0);
    setStatusMessage(i18n("Starting flash operation..."));
    setIsWriting(true);

    // Start writing in a separate thread
    QThread* thread = new QThread(this);
    m_writer->moveToThread(thread);
    
    connect(thread, &QThread::started, m_writer, &ImageWriter::writeImage);
    connect(m_writer, &ImageWriter::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    
    thread->start();
}

void FlashController::cancelFlashing()
{
    if (m_writer && m_isWriting) {
        m_writer->cancelWriting();
        setStatusMessage(i18n("Cancelling..."));
    }
}

void FlashController::onWriterProgress(int percent)
{
    setProgress(percent / 100.0);
    setStatusMessage(i18n("Writing: %1%", percent));
    qDebug() << "Progress " << percent;


}

void FlashController::onWriterSuccess(const QString& msg)
{
    setStatusMessage(msg.isEmpty() ? i18n("Flash completed successfully!") : msg);
    setProgress(1.0);
    emit flashCompleted();
}

void FlashController::onWriterError(const QString& msg)
{
    setErrorMessage(msg);
    setStatusMessage(i18n("Flash failed"));
    emit flashFailed(msg);
}

void FlashController::onWriterCancelled()
{
    setStatusMessage(i18n("Flash operation cancelled"));
    setProgress(0.0);
}

void FlashController::onWriterFinished()
{
    setIsWriting(false);
}

void FlashController::setIsWriting(bool writing)
{
    if (m_isWriting != writing) {
        m_isWriting = writing;
        emit isWritingChanged();
    }
}

void FlashController::setProgress(double progress)
{
    if (qAbs(m_progress - progress) > 0.001) {
        m_progress = progress;
        emit progressChanged();
    }
}

void FlashController::setStatusMessage(const QString& message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

void FlashController::setErrorMessage(const QString& message)
{
    if (m_errorMessage != message) {
        m_errorMessage = message;
        emit errorMessageChanged();
    }
}
