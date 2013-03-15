#ifndef MAINDIALOG_H
#define MAINDIALOG_H

#include <QDialog>

namespace Ui {
    class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();
    
private:
    Ui::MainDialog *ui;

protected:
    QString m_ImageFile;
    void enumFlashDevices();
    void preprocessImageFile(const QString& newImageFile);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

public slots:
    void openImageFile();
    void writeImageToDevice();
};


class UsbDevice
{
public:
    UsbDevice() : m_VisibleName("Unknown Device"), m_Volumes(""), m_Size(0), m_PhysicalDevice("") {}

    QString m_VisibleName;
    QString m_Volumes;
    qint64  m_Size;
    QString m_PhysicalDevice;
};

Q_DECLARE_METATYPE(UsbDevice*)

#endif // MAINDIALOG_H
