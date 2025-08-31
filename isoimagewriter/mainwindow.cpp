/*
    SPDX-FileCopyrightText: 2019 Farid Boudedja <farid.boudedja@gmail.com>, 2023 Jonathan Esk-Riddell <jr@jriddell.org>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "mainwindow.h"
#include "common.h"
#include "fetchisojob.h"
#include "imagewriter.h"
#include "isoimagewriter_debug.h"
#include "isoverifier.h"
#include "mainapplication.h"

#include "isolineedit.h"
#include <KFormat>
#include <KIconLoader>
#include <KLocalizedString>
#include <KPixmapSequence>
#include <QAction>
#include <QDropEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QStandardPaths>
#include <QThread>
#include <QTimer>
#include <QVBoxLayout>

#include <KPixmapSequenceLoader>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_lastOpenedDir()
    , m_isWriting(false)
    , m_enumFlashDevicesWaiting(false)
    , m_externalProgressBar(this)
{
    setupUi();
    setAcceptDrops(true);

    // Set initial directory
    m_lastOpenedDir = mApp->getInitialDir();
    // Get path to ISO image from command line args (if supplied)
    QUrl isoImagePath = mApp->getInitialImage();
    if (isoImagePath.isLocalFile()) {
        const QString path = isoImagePath.toLocalFile();
        m_lastOpenedDir = path.left(path.lastIndexOf('/'));
        preprocessIsoImage(path);
    } else {
        m_isoImageLineEdit->setText(isoImagePath.toString());
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

void MainWindow::showInputDialog(const QString &title, const QString &body)
{
    bool ok;
    QString text = QInputDialog::getText(this, title, body, QLineEdit::Normal, "", &ok);

    emit inputTextReady(ok, text);
}

void MainWindow::setupUi()
{
    // Logo
    QLabel *logoLabel = new QLabel;
    logoLabel->setPixmap(KIconLoader::global()->loadIcon("org.kde.isoimagewriter", KIconLoader::Desktop));
    logoLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *titleLabel = new QLabel;
    titleLabel->setTextFormat(Qt::RichText);
    titleLabel->setText(QStringLiteral("<h2 style='margin-bottom: 0;'>%1</h2>%2")
                            .arg(i18n("KDE ISO Image Writer"))
                            .arg(i18n("A quick and simple way to create a bootable USB drive.")));

    QHBoxLayout *headerHBoxLayout = new QHBoxLayout;
    headerHBoxLayout->addWidget(logoLabel);
    headerHBoxLayout->addWidget(titleLabel);

    m_centralStackedWidget = new QStackedWidget;
    m_centralStackedWidget->addWidget(createFormWidget());
    m_centralStackedWidget->addWidget(createConfirmWidget());
    m_centralStackedWidget->addWidget(createProgressWidget());
    m_centralStackedWidget->addWidget(createSuccessWidget());

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addLayout(headerHBoxLayout);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(m_centralStackedWidget);

    QWidget *centralWidget = new QWidget;
    centralWidget->setLayout(mainVBoxLayout);

    setCentralWidget(centralWidget);
}

QWidget *MainWindow::createFormWidget()
{
    // Form
    m_isoImageLineEdit = new IsoLineEdit;
    m_isoImageLineEdit->setReadOnly(true);
    m_isoImageLineEdit->setPlaceholderText(i18n("Path to ISO image…"));
    connect(m_isoImageLineEdit, &IsoLineEdit::clicked, this, &MainWindow::openIsoImage);

    m_isoImageSizeLabel = new QLabel;

    QAction *openIsoImageAction = m_isoImageLineEdit->addAction(QIcon::fromTheme("folder-open"), QLineEdit::TrailingPosition);
    connect(openIsoImageAction, &QAction::triggered, this, &MainWindow::openIsoImage);

    m_usbDriveComboBox = new QComboBox;

    m_createButton = new QPushButton(i18nc("@action:button", "Create"));
    connect(m_createButton, &QPushButton::clicked, this, &MainWindow::showConfirmMessage);

    m_busyLabel = new QLabel;
    m_busyWidget = new QWidget;
    m_busySpinner = new KPixmapSequenceOverlayPainter(this);
    m_busySpinner->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busySpinner->setWidget(m_busyWidget);
    m_busyWidget->setFixedSize(24, 24);

    QHBoxLayout *footerBoxLayout = new QHBoxLayout;
    footerBoxLayout->addWidget(m_busyWidget);
    footerBoxLayout->addWidget(m_busyLabel);
    footerBoxLayout->addWidget(m_createButton, 0, Qt::AlignRight);

    QHBoxLayout *isoImageLayout = new QHBoxLayout;
    isoImageLayout->addWidget(m_isoImageLineEdit);
    isoImageLayout->addWidget(m_isoImageSizeLabel);

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(new QLabel(i18n("ISO image:")));
    mainVBoxLayout->addLayout(isoImageLayout);
    mainVBoxLayout->addSpacing(5);
    mainVBoxLayout->addWidget(new QLabel(i18n("USB drive:")));
    mainVBoxLayout->addWidget(m_usbDriveComboBox);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addStretch();
    mainVBoxLayout->addLayout(footerBoxLayout);

    QWidget *formWidget = new QWidget;
    formWidget->setLayout(mainVBoxLayout);

    return formWidget;
}

QWidget *MainWindow::createConfirmWidget()
{
    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(64, 64)));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    static const QString overwriteMessage = i18n(
        "Everything on the USB drive will "
        "be overwritten."
        "\n\nDo you want to continue?");
    QLabel *messageLabel = new QLabel(overwriteMessage);
    connect(this, &MainWindow::downloadProgressChanged, messageLabel, [messageLabel, iconLabel, this] {
        iconLabel->setPixmap(QIcon::fromTheme("download").pixmap(QSize(64, 64)));
        messageLabel->setText(i18n("Downloading %1…", m_fetchIso->fetchUrl().toDisplayString()));
    });
    connect(this, &MainWindow::verificationResult, messageLabel, [messageLabel, iconLabel] {
        messageLabel->setText(overwriteMessage);
        iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(64, 64)));
    });

    QHBoxLayout *messageHBoxLayout = new QHBoxLayout;
    messageHBoxLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    messageHBoxLayout->addWidget(messageLabel, 0, Qt::AlignTop);

    QProgressBar *downloadProgressBar = new QProgressBar();
    downloadProgressBar->setVisible(false);
    downloadProgressBar->setFormat(i18nc("Progress percent value", "%p%"));
    downloadProgressBar->setMinimum(0);
    downloadProgressBar->setMaximum(100);
    connect(this, &MainWindow::downloadProgressChanged, downloadProgressBar, &QProgressBar::show);
    connect(this, &MainWindow::downloadProgressChanged, downloadProgressBar, &QProgressBar::setValue);

    QPushButton *abortButton = new QPushButton(i18nc("@action:button", "Abort"));
    connect(abortButton, &QPushButton::clicked, this, &MainWindow::hideWritingProgress);

    QPushButton *continueButton = new QPushButton(i18nc("@action:button", "Continue"));
    connect(continueButton, &QPushButton::clicked, this, &MainWindow::writeIsoImage);
    connect(this, &MainWindow::downloadProgressChanged, continueButton, [continueButton] {
        continueButton->setEnabled(false);
    });
    connect(this, &MainWindow::verificationResult, continueButton, [continueButton] {
        continueButton->setEnabled(true);
    });

    QHBoxLayout *buttonsHBoxLayout = new QHBoxLayout;
    buttonsHBoxLayout->addWidget(abortButton, 0, Qt::AlignLeft);
    buttonsHBoxLayout->addWidget(continueButton, 0, Qt::AlignRight);

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addLayout(messageHBoxLayout);
    mainVBoxLayout->addWidget(downloadProgressBar);
    mainVBoxLayout->addLayout(buttonsHBoxLayout);

    QWidget *confirmWidget = new QWidget;
    confirmWidget->setLayout(mainVBoxLayout);

    return confirmWidget;
}

QWidget *MainWindow::createProgressWidget()
{
    QLabel *messageLabel =
        new QLabel(i18n("Your USB drive is being created.\n\n"
                        "This may take some time depending "
                        "on the size of the ISO image file "
                        "and the transfer speed."));
    messageLabel->setWordWrap(true);

    m_progressBar = new QProgressBar;
    m_progressBar->setFormat(i18nc("Progress percent value", "%p%"));
    m_cancelButton = new QPushButton(i18nc("@action:button", "Cancel"));

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(messageLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    mainVBoxLayout->addWidget(m_progressBar);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(m_cancelButton, 0, Qt::AlignLeft);

    QWidget *progressWidget = new QWidget;
    progressWidget->setLayout(mainVBoxLayout);

    return progressWidget;
}

QWidget *MainWindow::createSuccessWidget()
{
    QLabel *messageLabel =
        new QLabel(i18n("Your live USB flash drive is now "
                        "complete and ready to use!"));
    messageLabel->setWordWrap(true);

    QLabel *successIconLabel = new QLabel();
    successIconLabel->setPixmap(QIcon::fromTheme("emblem-success").pixmap(QSize(64, 64)));

    QPushButton *backButton = new QPushButton(i18nc("@action:button", "Back"));
    connect(backButton, &QPushButton::clicked, this, &MainWindow::hideWritingProgress);

    QPushButton *closeButton = new QPushButton(i18nc("@action:button", "Close"));
    connect(closeButton, &QPushButton::clicked, this, &MainWindow::close);

    QHBoxLayout *buttonsHBoxLayout = new QHBoxLayout;
    buttonsHBoxLayout->addWidget(backButton, 0, Qt::AlignLeft);
    buttonsHBoxLayout->addWidget(closeButton, 0, Qt::AlignRight);

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(messageLabel, 0, Qt::AlignCenter);
    mainVBoxLayout->addWidget(successIconLabel, 0, Qt::AlignHCenter);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addLayout(buttonsHBoxLayout);

    QWidget *successWidget = new QWidget;
    successWidget->setLayout(mainVBoxLayout);

    return successWidget;
}

void MainWindow::preprocessIsoImage(const QString &isoImagePath)
{
    QFile file(isoImagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this,
                              "Error",
                              i18n("Failed to open the image file:") + "\n" + QDir::toNativeSeparators(isoImagePath) + "\n" + file.errorString());
        return;
    }

    m_isoImageSize = file.size();
    m_isoImagePath = isoImagePath;
    m_isoImageLineEdit->setText(QDir::toNativeSeparators(m_isoImagePath));
    m_isoImageSizeLabel->setText(KFormat().formatByteSize(m_isoImageSize));

    file.close();

#ifdef _USE_GPG
    // Verify ISO image
    m_busyLabel->setText(i18n("Verifying ISO image"));
    m_busyWidget->show();
    m_busySpinner->setSequence(KPixmapSequenceLoader::load("process-working", KIconLoader::SizeSmallMedium));
    m_busySpinner->start();

    IsoVerifier *isoVerifier = new IsoVerifier(m_isoImagePath);
    QThread *verifierThread = new QThread(this);

    connect(verifierThread, &QThread::started, isoVerifier, &IsoVerifier::verifyIso);
    connect(verifierThread, &QThread::finished, verifierThread, &QThread::deleteLater);

    connect(isoVerifier, &IsoVerifier::finished, verifierThread, &QThread::quit);
    connect(isoVerifier, &IsoVerifier::finished, isoVerifier, &IsoVerifier::deleteLater);
    connect(isoVerifier, &IsoVerifier::finished, this, &MainWindow::showIsoVerificationResult);
    connect(isoVerifier, &IsoVerifier::inputRequested, this, &MainWindow::showInputDialog);

    connect(this, &MainWindow::inputTextReady, isoVerifier, &IsoVerifier::verifyWithInputText);

    isoVerifier->moveToThread(verifierThread);
    verifierThread->start();
#endif

    // Enable the Write button (if there are USB flash disks present)
    m_createButton->setEnabled(m_usbDriveComboBox->count() > 0);
}

void MainWindow::cleanUp()
{
    // Delete all the allocated UsbDevice objects attached to the combobox
    for (int i = 0; i < m_usbDriveComboBox->count(); ++i) {
        delete m_usbDriveComboBox->itemData(i).value<UsbDevice *>();
    }
}

void MainWindow::enumFlashDevices()
{
    m_enumFlashDevicesWaiting = false;

    // Remember the currently selected device
    QString selectedDevice = "";
    int idx = m_usbDriveComboBox->currentIndex();
    if (idx >= 0) {
        UsbDevice *dev = m_usbDriveComboBox->itemData(idx).value<UsbDevice *>();
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
        for (int i = 0; i < m_usbDriveComboBox->count(); ++i) {
            UsbDevice *dev = m_usbDriveComboBox->itemData(i).value<UsbDevice *>();
            if (dev->m_PhysicalDevice == selectedDevice) {
                m_usbDriveComboBox->setCurrentIndex(i);
                break;
            }
        }

    // Update the Write button enabled/disabled state
    m_createButton->setEnabled(m_usbDriveComboBox->count() > 0 && m_isoImagePath != "");

    // Enable/disable the usb drive combobox
    if (m_usbDriveComboBox->count() < 1) {
        m_usbDriveComboBox->setEnabled(false);
        m_usbDriveComboBox->setEditable(true);
        m_usbDriveComboBox->lineEdit()->setPlaceholderText(i18n("Please plug in a USB drive"));
    } else {
        m_usbDriveComboBox->setEnabled(true);
        m_usbDriveComboBox->setEditable(false);
    }
}

void MainWindow::writeToDevice(bool zeroing)
{
    UsbDevice *selectedDevice = m_usbDriveComboBox->itemData(m_usbDriveComboBox->currentIndex()).value<UsbDevice *>();

    ImageWriter *writer = new ImageWriter(zeroing ? "" : m_isoImagePath, selectedDevice);
    QThread *writerThread = new QThread(this);

    // Connect start and end signals
    connect(writerThread, &QThread::started, writer, &ImageWriter::writeImage);
    // When writer finishes its job, quit the thread
    connect(writer, &ImageWriter::finished, writerThread, &QThread::quit);
    // Guarantee deleting the objects after completion
    connect(writer, &ImageWriter::finished, writer, &ImageWriter::deleteLater);
    connect(writerThread, &QThread::finished, writerThread, &QThread::deleteLater);
    // If the Cancel button is pressed, inform the writer to stop the operation
    // Using DirectConnection because the thread does not read its own event queue until completion
    connect(m_cancelButton, &QPushButton::clicked, writer, &ImageWriter::cancelWriting, Qt::DirectConnection);
    // Each time a block is written, update the progress bar
    connect(writer, &ImageWriter::progressChanged, this, &MainWindow::updateProgressBar);
    // Show the message about successful completion on success
    connect(writer, &ImageWriter::success, this, &MainWindow::showSuccessMessage);
    // Show error message if error is sent by the worker
    connect(writer, &ImageWriter::error, this, &MainWindow::showErrorMessage);
    // Silently return back to normal dialog form if the operation was cancelled
    connect(writer, &ImageWriter::cancelled, this, &MainWindow::hideWritingProgress);

    // Now start the writer thread
    writer->moveToThread(writerThread);
    writerThread->start();

    showWritingProgress();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    // Accept only files with ANSI or Unicode paths (Windows) and URIs (Linux)
    if (event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileName\"")
        || event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileNameW\"") || event->mimeData()->hasFormat("text/uri-list"))
        event->accept();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QString newImageFile = "";
    QByteArray droppedFileName;

    // First, try to use the Unicode file name
    droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileNameW\"");
    if (!droppedFileName.isEmpty()) {
        newImageFile = QString::fromWCharArray(reinterpret_cast<const wchar_t *>(droppedFileName.constData()));
    } else {
        // If failed, use the ANSI name with the local codepage
        droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileName\"");
        if (!droppedFileName.isEmpty()) {
            newImageFile = QString::fromLocal8Bit(droppedFileName.constData());
        } else {
            // And, finally, try the URI
            droppedFileName = event->mimeData()->data("text/uri-list");
            if (!droppedFileName.isEmpty()) {
                // If several files are dropped they are separated by newlines,
                // take the first file
                int newLineIndexLF = droppedFileName.indexOf('\n');
                int newLineIndex = droppedFileName.indexOf("\r\n");
                // Make sure both CRLF and LF are accepted
                if ((newLineIndexLF != -1) && (newLineIndexLF < newLineIndex))
                    newLineIndex = newLineIndexLF;
                if (newLineIndex != -1)
                    droppedFileName.truncate(newLineIndex);
                // Decode the file path from percent-encoding
                QUrl url = QUrl::fromEncoded(droppedFileName);
                if (url.isLocalFile())
                    newImageFile = url.toLocalFile();
            }
        }
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_isWriting) {
        const int answer = QMessageBox::question(this, i18n("Cancel?"), i18n("Writing is in progress, abort it?"));

        if (answer == QMessageBox::No)
            event->ignore();
    }
}

void MainWindow::addFlashDeviceCallback(void *cbParam, UsbDevice *device)
{
    auto usbDriveComboBox = (QComboBox *)cbParam;
    usbDriveComboBox->addItem(device->formatDisplayName(), QVariant::fromValue(device));
}

void MainWindow::openIsoImage()
{
    const QString filter = i18n("Disk Images (%1)", QString("*.iso *.bin *.img *.iso.gz *.iso.xz *.img.zstd *.img.gz *.img.zx *.img.zstd *.raw")) + ";;"
        + i18n("All Files (%1)", QString("*"));
    QUrl isoImageUrl =
        QFileDialog::getOpenFileUrl(this, i18n("Select image to flash"), QUrl::fromLocalFile(m_lastOpenedDir), filter, nullptr, QFileDialog::ReadOnly);
    openUrl(isoImageUrl);
}

void MainWindow::openUrl(const QUrl &url)
{
    if (url.isEmpty()) {
        return;
    }

    if (url.isLocalFile()) {
        const QString path = url.toLocalFile();
        m_lastOpenedDir = path.left(path.lastIndexOf('/'));
        preprocessIsoImage(path);
        return;
    } else {
        m_isoImageLineEdit->setText(url.toString());
    }

    delete m_fetchIso;
    m_fetchIso = new FetchIsoJob(this);
    connect(m_fetchIso, &FetchIsoJob::downloadProgressChanged, this, &MainWindow::downloadProgressChanged);
    connect(m_fetchIso, &FetchIsoJob::failed, this, &MainWindow::hideWritingProgress);
    connect(m_fetchIso, &FetchIsoJob::finished, this, [this](const QString &file) {
        m_isoImagePath = file;
        m_busySpinner->stop();
        preprocessIsoImage(file);
    });
    m_busyLabel->setText(i18n("Downloading ISO image"));
    m_busyWidget->show();
    m_busySpinner->setSequence(KPixmapSequenceLoader::load("process-working", KIconLoader::SizeSmallMedium));
    m_busySpinner->start();
    m_fetchIso->fetch(url);
}

void MainWindow::writeIsoImage()
{
    if (m_usbDriveComboBox->count() == 0 || m_isoImagePath == "")
        return;

    UsbDevice *selectedDevice = m_usbDriveComboBox->itemData(m_usbDriveComboBox->currentIndex()).value<UsbDevice *>();

    if (selectedDevice->m_Size == 0) {
        int warningReturn = QMessageBox::warning(this,
                                                 i18n("Unknown Disk Size"),
                                                 i18n("The selected disk is of unknown size, please check the image will fit before writing.\n\n"
                                                      "Image size: %1 (%2 b)",
                                                      KFormat().formatByteSize(m_isoImageSize),
                                                      m_isoImageSize),
                                                 QMessageBox::Ok | QMessageBox::Cancel);
        if (warningReturn != QMessageBox::Ok) {
            return;
        }
    } else if (m_isoImageSize > selectedDevice->m_Size) {
        QMessageBox::critical(this,
                              i18nc("@title:window", "Error"),
                              i18n("The image is larger than your selected device!\n\n"
                                   "Image size: %1 (%2 b)\n"
                                   "Disk size: %3 (%4 b)",
                                   KFormat().formatByteSize(m_isoImageSize),
                                   m_isoImageSize,
                                   KFormat().formatByteSize(selectedDevice->m_Size),
                                   selectedDevice->m_Size),
                              QMessageBox::Ok);
        return;
    }

    writeToDevice(false);
}

void MainWindow::updateProgressBar(int percent)
{
    m_progressBar->setValue(percent);
    m_externalProgressBar.SetProgressValue(percent);
}

void MainWindow::showWritingProgress()
{
    m_isWriting = true;

    // Do not accept dropped files while writing
    setAcceptDrops(false);

    // Display and customize the progress bar part
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);

    // Expose the progress bar state to the OS
    m_externalProgressBar.InitProgressBar(100);

    m_centralStackedWidget->setCurrentIndex(2);
}

void MainWindow::hideWritingProgress()
{
    m_isWriting = false;

    // Enable drag & drop
    setAcceptDrops(true);

    // Send a signal that progressbar is no longer present
    m_externalProgressBar.DestroyProgressBar();

    m_centralStackedWidget->setCurrentIndex(0);

    // If device list changed during writing update it now
    if (m_enumFlashDevicesWaiting)
        enumFlashDevices();
}

void MainWindow::showErrorMessage(const QString &message)
{
    m_externalProgressBar.ProgressSetError();

    QMessageBox::critical(this, i18nc("@title:window", "Error"), message);

    hideWritingProgress();
}

void MainWindow::showSuccessMessage()
{
    m_isWriting = false;

    // Do not accept dropped files
    setAcceptDrops(false);

    m_centralStackedWidget->setCurrentIndex(3);
}

void MainWindow::showConfirmMessage()
{
    openUrl(QUrl::fromUserInput(m_isoImageLineEdit->text(), {}, QUrl::AssumeLocalFile));

    // Do not accept dropped files
    setAcceptDrops(false);

    m_centralStackedWidget->setCurrentIndex(1);
}

void MainWindow::showIsoVerificationResult(IsoVerifier::VerifyResult verify, const QString &error)
{
    if (verify == IsoVerifier::VerifyResult::Successful) {
        m_busyLabel->setText(i18n("The ISO image is valid"));
        m_busySpinner->setSequence(KPixmapSequenceLoader::load("checkmark", KIconLoader::SizeSmallMedium));
    } else {
        m_busySpinner->setSequence(KPixmapSequenceLoader::load("error", KIconLoader::SizeSmallMedium));
        if (verify == IsoVerifier::VerifyResult::KeyNotFound) {
            m_busyLabel->setText(i18n("Could not find the key to verify the ISO image"));
        } else if (verify == IsoVerifier::VerifyResult::Failed) {
            m_busyLabel->setText(i18n("Could not verify ISO image"));
            QMessageBox::warning(this, i18n("ISO Verification failed"), error);
        }
    }
    Q_EMIT verificationResult(verify == IsoVerifier::VerifyResult::Successful);
}

#include "moc_mainwindow.cpp"
