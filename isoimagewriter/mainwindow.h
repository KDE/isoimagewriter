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

    void setupUi();
    QWidget* createFormWidget();
};

#endif // MAINWINDOW_H
