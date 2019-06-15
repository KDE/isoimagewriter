#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "usbdevice.h"

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);

public slots:
    void scheduleEnumFlashDevices();

private:
    QLineEdit *m_isoImageLineEdit;
    QComboBox *m_usbDriveComboBox;

    QString m_isoImagePath;
    quint64 m_isoImageSize;
    QString m_lastOpenedDir;
    bool m_isWriting;
    bool m_enumFlashDevicesWaiting;

    void setupUi();
    QWidget* createFormWidget();
    void preprocessIsoImage(const QString& isoImagePath);
    void cleanUp();
    void enumFlashDevices();

    static void addFlashDeviceCallback(void* cbParam, UsbDevice* device);

private slots:
    void openIsoImage();
};

#endif // MAINWINDOW_H
