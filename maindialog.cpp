#include "maindialog.h"
#include "ui_maindialog.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog)
{
    ui->setupUi(this);
    int currentHeight = this->size().height();
    setMaximumHeight(currentHeight);
    setMinimumHeight(currentHeight);
}

MainDialog::~MainDialog()
{
    delete ui;
}
