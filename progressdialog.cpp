#include <QKeyEvent>
#include <QMessageBox>

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

void ProgressDialog::cancelWriting()
{
    emit cancelled();
    reject();
}

void ProgressDialog::updateProgressBar(int increment)
{
    ui->progressBar->setValue(ui->progressBar->value() + increment);
}

void ProgressDialog::showErrorMessage(const QString& msg)
{
    QMessageBox::critical(
        this,
        "ROSA Image Writer",
        msg,
        QMessageBox::Ok
    );
}
