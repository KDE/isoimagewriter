/*
    SPDX-FileCopyrightText: 2019 Farid Boudedja <farid.boudedja@gmail.com>

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "usbdevice.h"
#include "externalprogressbar.h"
#include "isoverifier.h"

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <KPixmapSequenceOverlayPainter>

class FetchIsoJob;
class IsoLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

public slots:
    void scheduleEnumFlashDevices();
    void showInputDialog(const QString &title, const QString &body);

signals:
    void inputTextReady(bool ok, const QString &text);
    void downloadProgressChanged(int percentage);
    void verificationResult(bool verified);

private:
    QLabel *m_busyLabel;
    QWidget *m_busyWidget;
    QLabel *m_isoImageSizeLabel;
    IsoLineEdit *m_isoImageLineEdit;
    QComboBox *m_usbDriveComboBox;
    QPushButton *m_createButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QStackedWidget *m_centralStackedWidget;
    KPixmapSequenceOverlayPainter *m_busySpinner;
    FetchIsoJob *m_fetchIso = nullptr;

    QString m_isoImagePath;
    quint64 m_isoImageSize;
    QString m_lastOpenedDir;
    bool m_isWriting;
    bool m_enumFlashDevicesWaiting;
    ExternalProgressBar m_externalProgressBar;

    void setupUi();
    QWidget* createFormWidget();
    QWidget* createConfirmWidget();
    QWidget* createProgressWidget();
    QWidget* createSuccessWidget();
    void preprocessIsoImage(const QString& isoImagePath);
    void cleanUp();
    void enumFlashDevices();
    void writeToDevice(bool zeroing);

    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    static void addFlashDeviceCallback(void* cbParam, UsbDevice* device);

private slots:
    void openIsoImage();
    void writeIsoImage();
    void updateProgressBar(int increment);
    void showWritingProgress();
    void hideWritingProgress();
    void showErrorMessage(const QString &message);
    void showSuccessMessage();
    void showConfirmMessage();
    void showIsoVerificationResult(IsoVerifier::VerifyResult result, const QString &error);
    void openUrl(const QUrl &url);
};

#endif // MAINWINDOW_H
