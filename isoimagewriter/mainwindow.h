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
    MainWindow(QWidget *parent = nullptr);

public slots:
    void scheduleEnumFlashDevices();

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

#if defined(Q_OS_LINUX)
    void cancelWriting();
    void progressStep(KJob* job, unsigned long step);
    void progressStep(const QVariantMap &);
    void statusChanged(KAuth::Action::AuthStatus status);
    void finished(KJob* job);
#endif
};

#endif // MAINWINDOW_H
