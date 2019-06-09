#include "mainwindow.h"
#include "mainapplication.h"
#include "common.h"

#include <QLabel>
#include <QAction>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QStackedWidget>
#include <KFormat>
#include <KLocalizedString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_lastOpenedDir("")
{
    setupUi();

    m_lastOpenedDir = mApp->getInitialDir();
    // TODO: Use ISO image from command line args
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

    QAction *openIsoImageAction = m_isoImageLineEdit->addAction(
        QIcon::fromTheme("folder-open"), QLineEdit::TrailingPosition);
    connect(openIsoImageAction, &QAction::triggered, this, &MainWindow::openIsoImage);

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

void MainWindow::openIsoImage()
{
    const QString filter = i18n("Disk Images (%1)", QString("*.iso *.bin *.img"))
        + ";;" + i18n("All Files (%1)", QString("*"));
    QString isoImagePath = QFileDialog::getOpenFileName(this, "", m_lastOpenedDir, 
                                                        filter, nullptr,
                                                        QFileDialog::ReadOnly);
    if (!isoImagePath.isEmpty())
    {
        m_lastOpenedDir = isoImagePath.left(isoImagePath.lastIndexOf('/'));
        preprocessIsoImage(isoImagePath);
    }
}

void MainWindow::preprocessIsoImage(const QString& isoImagePath)
{
    QFile file(isoImagePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, "Error",
                              i18n("Failed to open the image file:")
                              + "\n" + QDir::toNativeSeparators(isoImagePath)
                              + "\n" + file.errorString());
        return;
    }

    m_isoImageSize = file.size();
    m_isoImagePath = isoImagePath;
    m_isoImageLineEdit->setText(QDir::toNativeSeparators(m_isoImagePath) + " ("
                                + KFormat().formatByteSize(m_isoImageSize) + ")");

    file.close();

    // TODO: Verify ISO image
}
