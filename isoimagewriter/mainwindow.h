#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QComboBox>

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);

private:
    QLineEdit *m_isoImageLineEdit;
    QComboBox *m_usbDriveComboBox;

    QString m_isoImagePath;
    quint64 m_isoImageSize;
    QString m_lastOpenedDir;

    void setupUi();
    QWidget* createFormWidget();
    void preprocessIsoImage(const QString& isoImagePath);

private slots:
    void openIsoImage();
};

#endif // MAINWINDOW_H
