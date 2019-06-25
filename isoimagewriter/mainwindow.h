#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "usbdevice.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QStackedWidget>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void scheduleEnumFlashDevices();

private:
    QLineEdit *m_isoImageLineEdit;
    QComboBox *m_usbDriveComboBox;
    QPushButton *m_createButton;
    QPushButton *m_cancelButton;
    QProgressBar *m_progressBar;
    QStackedWidget *m_centralStackedWidget;

    QString m_isoImagePath;
    quint64 m_isoImageSize;
    QString m_lastOpenedDir;
    bool m_isWriting;
    bool m_enumFlashDevicesWaiting;

    void setupUi();
    QWidget* createFormWidget();
    QWidget* createConfirmWidget();
    QWidget* createProgressWidget();
    QWidget* createSuccessWidget();
    void preprocessIsoImage(const QString& isoImagePath);
    void cleanUp();
    void enumFlashDevices();
    void writeToDevice(bool zeroing);

    static void addFlashDeviceCallback(void* cbParam, UsbDevice* device);

private slots:
    void openIsoImage();
    void writeIsoImage();
    void updateProgressBar(int increment);
    void showWritingProgress(int maxValue);
    void hideWritingProgress();
    void showErrorMessage(const QString &message);
};

#endif // MAINWINDOW_H
