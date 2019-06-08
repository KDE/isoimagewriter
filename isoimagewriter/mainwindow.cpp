#include "mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <KLocalizedString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

void MainWindow::setupUi()
{
    QStackedWidget *stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(createFormWidget());

    setCentralWidget(stackedWidget);
}

QWidget* MainWindow::createFormWidget()
{    // Logo
    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(QIcon::fromTheme("drive-removable-media").pixmap(QSize(50, 50)));

    QLabel *titleLabel = new QLabel;
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText("<h2 style='margin-bottom: 0;'>KDE ISO Image Writer</h2>"
                        "A quick and simple way to create a bootable USB drive.");

    QHBoxLayout *headerHBoxLayout = new QHBoxLayout;
    headerHBoxLayout->addWidget(logoLabel);
    headerHBoxLayout->addWidget(titleLabel);

    // Form
    m_isoImageLineEdit = new QLineEdit;
    m_isoImageLineEdit->setReadOnly(true);
    m_isoImageLineEdit->setPlaceholderText(i18n("Path to ISO image..."));

    m_usbDriveComboBox = new QComboBox;

    QPushButton *createButton = new QPushButton(i18n("Create"));

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addLayout(headerHBoxLayout);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(new QLabel(i18n("Write this ISO image:")));
    mainVBoxLayout->addWidget(m_isoImageLineEdit);
    mainVBoxLayout->addSpacing(5);
    mainVBoxLayout->addWidget(new QLabel(i18n("To this USB drive:")));
    mainVBoxLayout->addWidget(m_usbDriveComboBox);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(createButton, 0, Qt::AlignRight);

    QWidget *formWidget = new QWidget;
    formWidget->setLayout(mainVBoxLayout);

    return formWidget;
}
