#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
    class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProgressDialog(int maxValue, QWidget *parent = 0);
    ~ProgressDialog();
    
private:
    Ui::ProgressDialog *ui;

protected:
    void keyPressEvent(QKeyEvent* keyEvent);

signals:
    void cancelled();

public slots:
    void cancelWriting();
    void updateProgressBar(int increment);
};

#endif // PROGRESSDIALOG_H
