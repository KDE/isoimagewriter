#include <QKeyEvent>

#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(int maxValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    setWindowFlags((windowFlags() | Qt::MSWindowsFixedSizeDialogHint | Qt::CustomizeWindowHint) & ~Qt::WindowTitleHint & ~Qt::WindowCloseButtonHint & ~Qt::WindowContextHelpButtonHint);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(maxValue);
    ui->progressBar->setValue(0);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::keyPressEvent(QKeyEvent* keyEvent)
{
    if (keyEvent->key() == Qt::Key_Escape)
        return;
    QDialog::keyPressEvent(keyEvent);
}
