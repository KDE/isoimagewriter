/*
 * Copyright 2019 Farid Boudedja <farid.boudedja@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "usbdevice.h"
#include "externalprogressbar.h"

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>
#include <KPixmapSequenceOverlayPainter>

#if defined(Q_OS_LINUX)
#include <KAuth>
#endif

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

private:
    QLabel *m_busyLabel;
    QWidget *m_busyWidget;
    QLineEdit *m_isoImageLineEdit;
    QComboBox *m_usbDriveComboBox;
    QPushButton *m_createButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QStackedWidget *m_centralStackedWidget;
    KPixmapSequenceOverlayPainter *m_busySpinner;

    QString m_isoImagePath;
    quint64 m_isoImageSize;
    QString m_lastOpenedDir;
    bool m_isWriting;
    bool m_enumFlashDevicesWaiting;
    ExternalProgressBar m_externalProgressBar;

#if defined(Q_OS_LINUX)
    KAuth::ExecuteJob *m_job;
#endif

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
    void showWritingProgress(int maxValue);
    void hideWritingProgress();
    void showErrorMessage(const QString &message);
    void showSuccessMessage();
    void showConfirmMessage();
    void showIsoVerificationResult(const bool &isIsoValid, const QString &error);

#if defined(Q_OS_LINUX)
    void cancelWriting();
    void progressStep(KJob* job, unsigned long step);
    void progressStep(const QVariantMap &);
    void statusChanged(KAuth::Action::AuthStatus status);
    void finished(KJob* job);
#endif
};

#endif // MAINWINDOW_H
