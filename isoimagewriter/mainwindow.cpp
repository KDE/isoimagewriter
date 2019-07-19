#include "mainwindow.h"
#include "mainapplication.h"
#include "common.h"
#include "imagewriter.h"
#include "isoimagewriter_debug.h"

#include <QLabel>
#include <QTimer>
#include <QAction>
#include <QThread>
#include <QMimeData>
#include <QDropEvent>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <KFormat>
#include <KLocalizedString>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      m_lastOpenedDir(""),
      m_isWriting(false),
      m_enumFlashDevicesWaiting(false),
      m_externalProgressBar(this)
{
    setupUi();
    setAcceptDrops(true);

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

    m_createButton = new QPushButton(i18n("Create"));
    m_createButton->setEnabled(false);
    connect(m_createButton, &QPushButton::clicked, this, &MainWindow::showConfirmMessage);

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(new QLabel(i18n("Write this ISO image:")));
    mainVBoxLayout->addWidget(m_isoImageLineEdit);
    mainVBoxLayout->addSpacing(5);
    mainVBoxLayout->addWidget(new QLabel(i18n("To this USB drive:")));
    mainVBoxLayout->addWidget(m_usbDriveComboBox);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addStretch();
    mainVBoxLayout->addWidget(m_createButton, 0, Qt::AlignRight);

    QWidget *formWidget = new QWidget;
    formWidget->setLayout(mainVBoxLayout);

    return formWidget;
}

QWidget* MainWindow::createConfirmWidget()
{
    QLabel *iconLabel = new QLabel;
    iconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(64, 64)));
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel *messageLabel = new QLabel(i18n("Everything on the USB drive will "
                                           "be overwritten."
                                           "\n\nDo you want to continue?"));

    QHBoxLayout *messageHBoxLayout = new QHBoxLayout;
    messageHBoxLayout->addWidget(iconLabel, 0, Qt::AlignTop);
    messageHBoxLayout->addWidget(messageLabel, 0, Qt::AlignTop);

    QPushButton *abortButton = new QPushButton(i18n("Abort"));
    connect(abortButton, &QPushButton::clicked, this, &MainWindow::hideWritingProgress);

    QPushButton *continueButton = new QPushButton(i18n("Continue"));
    connect(continueButton, &QPushButton::clicked, this, &MainWindow::writeIsoImage);

    QHBoxLayout *buttonsHBoxLayout = new QHBoxLayout;
    buttonsHBoxLayout->addWidget(abortButton, 0, Qt::AlignLeft);
    buttonsHBoxLayout->addWidget(continueButton, 0, Qt::AlignRight);

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addLayout(messageHBoxLayout);
    mainVBoxLayout->addLayout(buttonsHBoxLayout);

    QWidget *confirmWidget = new QWidget;
    confirmWidget->setLayout(mainVBoxLayout);

    return confirmWidget;
}

QWidget* MainWindow::createProgressWidget()
{
    QLabel *messageLabel = new QLabel(i18n("Your USB drive is being created.\n\n"
                                           "This may take some time depending "
                                           "on the size of the ISO image file "
                                           "and the transfer speed."));
    messageLabel->setWordWrap(true);

    m_progressBar = new QProgressBar;
    m_cancelButton = new QPushButton(i18n("Cancel"));

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(messageLabel, 0, Qt::AlignTop | Qt::AlignHCenter);
    mainVBoxLayout->addWidget(m_progressBar);
    mainVBoxLayout->addSpacing(15);
    mainVBoxLayout->addWidget(m_cancelButton, 0, Qt::AlignLeft);

    QWidget *progressWidget = new QWidget;
    progressWidget->setLayout(mainVBoxLayout);

    return progressWidget;
}

QWidget* MainWindow::createSuccessWidget()
{
    QLabel *messageLabel = new QLabel(i18n("Your live USB flash drive is now "
                                           "complete and ready to use!"));
    messageLabel->setWordWrap(true);

    QLabel *successIconLabel = new QLabel();
    successIconLabel->setPixmap(QIcon::fromTheme("emblem-success").pixmap(QSize(64, 64)));

    QVBoxLayout *mainVBoxLayout = new QVBoxLayout;
    mainVBoxLayout->addWidget(messageLabel, 0, Qt::AlignCenter);
    mainVBoxLayout->addWidget(successIconLabel, 0, Qt::AlignHCenter);
    mainVBoxLayout->addSpacing(15);

    QWidget *successWidget = new QWidget;
    successWidget->setLayout(mainVBoxLayout);

    return successWidget;
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

    // Enable the Write button (if there are USB flash disks present)
    m_createButton->setEnabled(m_usbDriveComboBox->count() > 0);
}

void MainWindow::cleanUp()
{
    // Delete all the allocated UsbDevice objects attached to the combobox
    for (int i = 0; i < m_usbDriveComboBox->count(); ++i)
    {
        delete m_usbDriveComboBox->itemData(i).value<UsbDevice*>();
    }
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
    m_createButton->setEnabled(m_usbDriveComboBox->count() > 0
                               && m_isoImagePath != "");
    // Update the Clear button enabled/disabled state
    // m_clearButton->setEnabled(m_usbDriveComboBox->count() > 0);
}

void MainWindow::writeToDevice(bool zeroing)
{
    UsbDevice* selectedDevice = m_usbDriveComboBox->itemData(
        m_usbDriveComboBox->currentIndex()).value<UsbDevice*>();

// Use KAuth to get required previleges in supported platforms
#if defined(Q_OS_LINUX)
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::cancelWriting);

    KAuth::Action action("org.kde.isoimagewriter.write");
    action.setHelperId("org.kde.isoimagewriter");

    QVariantMap args;
    args[QStringLiteral("zeroing")] = QVariant(zeroing);
    args[QStringLiteral("imagefile")] = m_isoImagePath;
    args[QStringLiteral("usbdevice_visiblename")] = selectedDevice->m_VisibleName;
    args[QStringLiteral("usbdevice_volumes")] = selectedDevice->m_Volumes[0];
    args[QStringLiteral("usbdevice_size")] = QString("%1").arg(selectedDevice->m_Size);
    args[QStringLiteral("usbdevice_sectorsize")] = selectedDevice->m_SectorSize;
    args[QStringLiteral("usbdevice_physicaldevice")] = selectedDevice->m_PhysicalDevice;

    action.setArguments(args);
    action.setTimeout(3600000); // an hour

    m_job = action.execute();

    connect(m_job, SIGNAL(percent(KJob*,ulong)), this, SLOT(progressStep(KJob*,ulong)), Qt::DirectConnection);
    connect(m_job, SIGNAL(newData(QVariantMap)), this, SLOT(progressStep(QVariantMap)));
    connect(m_job, &KAuth::ExecuteJob::statusChanged, this, &MainWindow::statusChanged);
    connect(m_job, &KAuth::ExecuteJob::result, this, &MainWindow::finished);

    m_job->start();
#else
    ImageWriter* writer = new ImageWriter(zeroing ? "" : m_isoImagePath, selectedDevice);
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
    connect(writer, &ImageWriter::blockWritten, this, &MainWindow::updateProgressBar);
    // Show the message about successful completion on success
    connect(writer, &ImageWriter::success, this, &MainWindow::showSuccessMessage);
    // Show error message if error is sent by the worker
    connect(writer, &ImageWriter::error, this, &MainWindow::showErrorMessage);
    // Silently return back to normal dialog form if the operation was cancelled
    connect(writer, &ImageWriter::cancelled, this, &MainWindow::hideWritingProgress);

    // Now start the writer thread
    writer->moveToThread(writerThread);
    writerThread->start();
#endif

    showWritingProgress(alignNumberDiv((zeroing ? DEFAULT_UNIT : m_isoImageSize), DEFAULT_UNIT));
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    // Accept only files with ANSI or Unicode paths (Windows) and URIs (Linux)
    if (event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileName\"") ||
        event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileNameW\"") ||
        event->mimeData()->hasFormat("text/uri-list"))
        event->accept();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    QString newImageFile = "";
    QByteArray droppedFileName;

    // First, try to use the Unicode file name
    droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileNameW\"");
    if (!droppedFileName.isEmpty()) {
        newImageFile = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(droppedFileName.constData()));
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
                    droppedFileName = droppedFileName.left(newLineIndex);
                // Decode the file path from percent-encoding
                QUrl url = QUrl::fromEncoded(droppedFileName);
                if (url.isLocalFile())
                    newImageFile = url.toLocalFile();
            }
        }
    }

    if (!newImageFile.isEmpty()) {
        // If something was really received update the information
        preprocessIsoImage(newImageFile);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (m_isWriting)
    {
        const int answer = QMessageBox::question(
            this,
            i18n("Cancel?"),
            i18n("Writing is in progress, abort it?"));

        if (answer == QMessageBox::No)
            event->ignore();
    }
}

void MainWindow::addFlashDeviceCallback(void* cbParam, UsbDevice* device)
{
    auto usbDriveComboBox = (QComboBox*)cbParam;
    usbDriveComboBox->addItem(device->formatDisplayName(),
                              QVariant::fromValue(device));
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

void MainWindow::writeIsoImage()
{
    if (m_usbDriveComboBox->count() == 0 || m_isoImagePath == "")
        return;

    UsbDevice* selectedDevice = m_usbDriveComboBox->itemData(
        m_usbDriveComboBox->currentIndex()).value<UsbDevice*>();

    if (m_isoImageSize > selectedDevice->m_Size)
    {
        QMessageBox::critical(
            this,
            i18n("Error"),
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

void MainWindow::updateProgressBar(int increment)
{
    int newValue = m_progressBar->value() + increment;
    m_progressBar->setValue(newValue);
    m_externalProgressBar.SetProgressValue(newValue);
}

void MainWindow::showWritingProgress(int maxValue)
{
    m_isWriting = true;

    // Do not accept dropped files while writing
    setAcceptDrops(false);

    // Display and customize the progress bar part
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(maxValue);
    m_progressBar->setValue(0);

    // Expose the progress bar state to the OS
    m_externalProgressBar.InitProgressBar(maxValue);

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

    QMessageBox::critical(this, i18n("Error"), message);

    hideWritingProgress();
}

void MainWindow::showSuccessMessage()
{
    // Do not accept dropped files
    setAcceptDrops(false);

    m_centralStackedWidget->setCurrentIndex(3);
}

void MainWindow::showConfirmMessage()
{
    // Do not accept dropped files
    setAcceptDrops(false);

    m_centralStackedWidget->setCurrentIndex(1);
}

#if defined(Q_OS_LINUX)
void MainWindow::cancelWriting() {
    qCDebug(ISOIMAGEWRITER_LOG) << "cancelWriting()";
    m_job->kill();
    qCDebug(ISOIMAGEWRITER_LOG) << "cancelWriting() done";
}

void MainWindow::progressStep(KJob* job, unsigned long step) {
    Q_UNUSED(job)
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep %() " << step;
    updateProgressBar(step);
}

void MainWindow::progressStep(const QVariantMap & data) {
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep(QVariantMap) ";// << step;
    if (data[QStringLiteral("progress")].isValid()) {
        int step = data[QStringLiteral("progress")].toInt();
        updateProgressBar(step);
    } else if (data[QStringLiteral("error")].isValid()) {
        showErrorMessage(data[QStringLiteral("error")].toString());
    } else if (data[QStringLiteral("success")].isValid()) {
        showSuccessMessage();
    }
}

void MainWindow::statusChanged(KAuth::Action::AuthStatus status) {
    qCDebug(ISOIMAGEWRITER_LOG) << "statusChanged: " << status;
}

void MainWindow::finished(KJob* job) {
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job->error();
    KAuth::ExecuteJob *job2 = (KAuth::ExecuteJob *)job;
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job2->data();

    if (job2->data()[QStringLiteral("success")].isValid())
        showSuccessMessage();
}
#endif
