#include "mainwindow.h"
#include "mainapplication.h"
#include "common.h"

#include <QLabel>
#include <QTimer>
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
      m_lastOpenedDir(""),
      m_isWriting(false),
      m_enumFlashDevicesWaiting(false)
{
    setupUi();

    // Set initial directory
    m_lastOpenedDir = mApp->getInitialDir();
    // Get path to ISO image from command line args (if supplied)
    QString isoImagePath = mApp->getInitialImage();
    if (!isoImagePath.isEmpty())
    {
        if (isoImagePath.left(7) == "file://")
            isoImagePath = QUrl(isoImagePath).toLocalFile();

        if (!isoImagePath.isEmpty())
        {
            isoImagePath = QDir(isoImagePath).absolutePath();
            // Update the default open dir
            m_lastOpenedDir = isoImagePath.left(isoImagePath.lastIndexOf('/'));
            preprocessIsoImage(isoImagePath);
        }
    }

    // Load the list of USB flash devices
    QTimer::singleShot(0, this, &MainWindow::enumFlashDevices);
}

void MainWindow::scheduleEnumFlashDevices()
{
    if (m_isWriting)
        m_enumFlashDevicesWaiting = true;
    else
        enumFlashDevices();
}

void MainWindow::setupUi()
{
    // Logo
    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(QIcon::fromTheme("drive-removable-media").pixmap(QSize(50, 50)));
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *titleLabel = new QLabel;
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText("<h2 style='margin-bottom: 0;'>KDE ISO Image Writer</h2>"
                        "A quick and simple way to create a bootable USB drive.");

    QHBoxLayout *headerHBoxLayout = new QHBoxLayout;
    headerHBoxLayout->addWidget(logoLabel);
    headerHBoxLayout->addWidget(titleLabel);

    QStackedWidget *stackedWidget = new QStackedWidget;
    stackedWidget->addWidget(createFormWidget());

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addLayout(headerHBoxLayout);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(stackedWidget);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainVBoxLayout);

    setCentralWidget(centralWidget);
}

QWidget* MainWindow::createFormWidget()
{
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
    mainVBoxLayout->addWidget(new QLabel(i18n("Write this ISO image:")));
    mainVBoxLayout->addWidget(m_isoImageLineEdit);
    mainVBoxLayout->addSpacing(5);
    mainVBoxLayout->addWidget(new QLabel(i18n("To this USB drive:")));
    mainVBoxLayout->addWidget(m_usbDriveComboBox);
    mainVBoxLayout->addStretch(15);
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

void MainWindow::cleanUp()
{
    // Delete all the allocated UsbDevice objects attached to the combobox
    for (int i = 0; i < m_usbDriveComboBox->count(); ++i)
    {
        delete m_usbDriveComboBox->itemData(i).value<UsbDevice*>();
    }
}

void MainWindow::addFlashDeviceCallback(void* cbParam, UsbDevice* device)
{
    auto usbDriveComboBox = (QComboBox*)cbParam;
    usbDriveComboBox->addItem(device->formatDisplayName(),
                              QVariant::fromValue(device));
}

void MainWindow::enumFlashDevices()
{
    m_enumFlashDevicesWaiting = false;

    // Remember the currently selected device
    QString selectedDevice = "";
    int idx = m_usbDriveComboBox->currentIndex();
    if (idx >= 0)
    {
        UsbDevice* dev = m_usbDriveComboBox->itemData(idx).value<UsbDevice*>();
        selectedDevice = dev->m_PhysicalDevice;
    }

    // Remove the existing entries
    cleanUp();
    m_usbDriveComboBox->clear();

    // Disable the combobox
    m_usbDriveComboBox->setEnabled(false);

    // Add the USB flash devices to the combobox
    platformEnumFlashDevices(addFlashDeviceCallback, m_usbDriveComboBox);

    // Restore the previously selected device (if present)
    if (!selectedDevice.isEmpty())
        for (int i = 0; i < m_usbDriveComboBox->count(); ++i)
        {
            UsbDevice* dev = m_usbDriveComboBox->itemData(i).value<UsbDevice*>();
            if (dev->m_PhysicalDevice == selectedDevice)
            {
                m_usbDriveComboBox->setCurrentIndex(i);
                break;
            }
        }

    // Re-enable the combobox
    m_usbDriveComboBox->setEnabled(true);
    // Update the Write button enabled/disabled state
    // m_writeButton->setEnabled((m_usbDriveComboBox->count() > 0) && (m_isoImagePath != ""));
    // Update the Clear button enabled/disabled state
    // m_clearButton->setEnabled(m_usbDriveComboBox->count() > 0);
}
