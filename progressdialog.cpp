#include "progressdialog.h"
#include "ui_progressdialog.h"

ProgressDialog::ProgressDialog(int maxValue, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(maxValue);
    ui->progressBar->setValue(0);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}
