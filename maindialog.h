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
    quint64 m_ImageSize;
    QString m_LastOpenedDir;
    void preprocessImageFile(const QString& newImageFile);

    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

public slots:
    void openImageFile();
    void enumFlashDevices();
    void writeImageToDevice();
    void showWritingProgress();
    void hideWritingProgress();
    void updateProgressBar(int increment);
    void showSuccessMessage();
    void showErrorMessage(QString msg);
};


class UsbDevice
{
public:
    UsbDevice() : m_VisibleName("Unknown Device"), m_Volumes(), m_Size(0), m_PhysicalDevice("") {}

    QString     m_VisibleName;
    QStringList m_Volumes;
    quint64     m_Size;
    QString     m_PhysicalDevice;
};

Q_DECLARE_METATYPE(UsbDevice*)

#endif // MAINDIALOG_H
