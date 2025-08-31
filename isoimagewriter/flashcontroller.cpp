/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <QDebug>
#include <QThread>

#include "flashcontroller.h"

FlashController::FlashController(QObject *parent)
    : QObject(parent)
    , m_writer(nullptr)
    , m_thread(nullptr)
    , m_isWriting(false)
    , m_progress(0.0)
{
}

FlashController::~FlashController()
{
    // Properly cleanup thread and writer
    if (m_writer && m_thread && m_thread->isRunning()) {
        qDebug() << "FlashController::~FlashController: Cleaning up running thread";
        m_writer->cancelWriting();

        // Wait for thread to finish properly
        m_thread->quit();
        if (!m_thread->wait(5000)) { // Wait up to 5 seconds
            qWarning() << "FlashController::~FlashController: Thread did not finish gracefully, terminating";
            m_thread->terminate();
            m_thread->wait(1000);
        }
    }

    if (m_writer) {
        m_writer->deleteLater();
        m_writer = nullptr;
    }

    if (m_thread) {
        m_thread->deleteLater();
        m_thread = nullptr;
    }
}

void FlashController::startFlashing(const QString &isoPath, UsbDevice *device)
{
    if (m_isWriting || !device) {
        qDebug() << "FlashController::startFlashing: Invalid state - isWriting:" << m_isWriting << "device:" << device;
        return;
    }

    // Validate device has a physical device path
    if (device->physicalDevice().isEmpty()) {
        qDebug() << "FlashController::startFlashing: Device has no physical device path";
        setErrorMessage(i18n("Invalid device selected"));
        return;
    }

    qDebug() << "FlashController::startFlashing: Starting flash of" << isoPath << "to" << device->physicalDevice();

    // Clean up any previous writer and thread
    if (m_writer) {
        m_writer->deleteLater();
        m_writer = nullptr;
    }

    if (m_thread) {
        if (m_thread->isRunning()) {
            qDebug() << "FlashController::startFlashing: Waiting for previous thread to finish";
            m_thread->quit();
            if (!m_thread->wait(3000)) {
                qWarning() << "FlashController::startFlashing: Previous thread did not finish, terminating";
                m_thread->terminate();
                m_thread->wait(1000);
            }
        }
        m_thread->deleteLater();
        m_thread = nullptr;
    }

    // Create new ImageWriter and thread
    m_writer = new ImageWriter(isoPath, device);
    m_thread = new QThread(this);

    // Connect signals
    connect(m_writer, &ImageWriter::progressChanged, this, &FlashController::onWriterProgress);
    connect(m_writer, &ImageWriter::success, this, &FlashController::onWriterSuccess);
    connect(m_writer, &ImageWriter::error, this, &FlashController::onWriterError);
    connect(m_writer, &ImageWriter::cancelled, this, &FlashController::onWriterCancelled);
    connect(m_writer, &ImageWriter::finished, this, &FlashController::onWriterFinished);

    // Reset state
    setErrorMessage("");
    setProgress(0.0);
    setStatusMessage(i18n("Starting flash operation…"));
    setIsWriting(true);

    // Start writing in a separate thread
    m_writer->moveToThread(m_thread);

    connect(m_thread, &QThread::started, m_writer, &ImageWriter::writeImage);
    connect(m_writer, &ImageWriter::finished, m_thread, &QThread::quit);

    // Use a safer cleanup approach - don't auto-delete the thread
    connect(m_thread, &QThread::finished, this, [this]() {
        if (m_thread) {
            m_thread->deleteLater();
            m_thread = nullptr;
        }
    });

    m_thread->start();
}

void FlashController::cancelFlashing()
{
    if (m_writer && m_isWriting) {
        qDebug() << "FlashController::cancelFlashing: Cancelling flash operation";
        m_writer->cancelWriting();
        setStatusMessage(i18n("Cancelling…"));

        // The thread will be cleaned up when the writer finishes
        // Don't force quit here as it can cause the "thread destroyed while running" error
    }
}

void FlashController::onWriterProgress(int percent)
{
    setProgress(percent / 100.0);
    setStatusMessage(i18n("Writing: %1%", percent));
    qDebug() << "Progress " << percent;
}

void FlashController::onWriterSuccess(const QString &msg)
{
    setStatusMessage(msg.isEmpty() ? i18n("Flash completed successfully!") : msg);
    setProgress(1.0);
    emit flashCompleted();
}

void FlashController::onWriterError(const QString &msg)
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

void FlashController::setStatusMessage(const QString &message)
{
    if (m_statusMessage != message) {
        m_statusMessage = message;
        emit statusMessageChanged();
    }
}

void FlashController::setErrorMessage(const QString &message)
{
    if (m_errorMessage != message) {
        m_errorMessage = message;
        emit errorMessageChanged();
    }
}
