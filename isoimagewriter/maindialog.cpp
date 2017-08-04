/*
 * Copyright 2016 ROSA
 * Copyright 2017 Jonathan Riddell <jr@jriddell.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

////////////////////////////////////////////////////////////////////////////////
// Implementation of MainDialog

#include <KIconLoader>
#include <KPixmapSequence>

#include <QMessageBox>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QLocale>
#include <QThread>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QDebug>
#include <QLoggingCategory>
#include <QMovie>

#include "common.h"
#include "mainapplication.h"
#include "maindialog.h"
#include "ui_maindialog.h"
#include "imagewriter.h"
#include "usbdevice.h"
#include "isoimagewriter_debug.h"
#include "verifyneoniso.h"
#include "verifynetrunneriso.h"
#include "verifykubuntuiso.h"
#include "verifyarchiso.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog),
    m_ImageFile(""),
    m_ImageSize(0),
    m_LastOpenedDir(""),
    m_IsWriting(false),
    m_EnumFlashDevicesWaiting(false),
    m_ExtProgressBar(this)
{
#if defined(Q_OS_MAC)
    // Quick hack for OS X to avoid hiding our main dialog when inactive
    // The Qt::WA_MacAlwaysShowToolWindow attribute does not work (see QTBUG-29816)
    void disableHideOnDeactivate(WId wid);
    disableHideOnDeactivate(winId());
#endif

    ui->setupUi(this);

#if defined(ROSA_BRANDING)
    // Compile with -DROSA_BRANDING=On to use the ROSA name
    setWindowTitle("ROSA Image Writer");
    ui->logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::AppDataLocation, "logo-rosa.png")));
#else
    ui->logo->setPixmap(QIcon::fromTheme("drive-removable-media").pixmap(QSize(50, 50)));
#endif
    ui->introLabel->setText(i18n("Select an ISO image file to write to a USB disk"));
    ui->imageSelectButton->setIcon(QIcon::fromTheme("folder-open"));
    ui->deviceRefreshButton->setIcon(QIcon::fromTheme("view-refresh"));
    ui->verificationResultLabel->hide();
    m_writeButton = ui->buttonBox->button(QDialogButtonBox::Yes);
    m_clearButton = ui->buttonBox->button(QDialogButtonBox::Reset);
    m_cancelButton = ui->buttonBox->button(QDialogButtonBox::Cancel);

    m_clearButton->setText(i18n("Clear USB Disk"));
    m_writeButton->setText(i18n("Write"));
    connect(m_writeButton, &QPushButton::clicked, this, &MainDialog::writeImageToDevice);
    m_clearButton->setText(i18n("Wipe USB Disk"));
    connect(m_clearButton, &QPushButton::clicked, this, &MainDialog::clearDevice);
    m_cancelButton->hide();
    // Remove the Context Help button and add the Minimize button to the titlebar
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint) & ~Qt::WindowContextHelpButtonHint);
    // Start in the "idle" mode
    hideWritingProgress();
    // Change default open dir
    m_LastOpenedDir = mApp->getInitialDir();
    // Get path to ISO from command line (if supplied)
    QString newImageFile = mApp->getInitialImage();
    if (!newImageFile.isEmpty())
    {
        if (newImageFile.left(7) == "file://")
            newImageFile = QUrl(newImageFile).toLocalFile();
        if (newImageFile != "")
        {
            newImageFile = QDir(newImageFile).absolutePath();
            // Update the default open dir
            m_LastOpenedDir = newImageFile.left(newImageFile.lastIndexOf('/'));
            preprocessImageFile(newImageFile);
        }
    }
    // Load the list of USB flash disks
    enumFlashDevices();
    // TODO: Increase the dialog display speed by showing it with the empty list and enumerating devices
    // in the background (dialog disabled, print "please wait")
    // TODO: Use dialog disabling also for manual refreshing the list
}

MainDialog::~MainDialog()
{
    cleanup();
    delete ui;
}

// Retrieves information about the selected file and displays it in the dialog
void MainDialog::preprocessImageFile(const QString& newImageFile)
{
    QFile f(newImageFile);
    if (!f.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(
            this,
            ApplicationTitle,
            i18n("Failed to open the image file:") + "\n" +
            QDir::toNativeSeparators(newImageFile) + "\n" +
            f.errorString()
        );
        return;
    }
    m_ImageSize = f.size();
    f.close();
    m_ImageFile = newImageFile;
    ui->imageEdit->setText(QDir::toNativeSeparators(m_ImageFile) + " " + i18n("(%1 MiB)", QString::number(alignNumberDiv(m_ImageSize, DEFAULT_UNIT))));

    m_busyWidget = new KPixmapSequenceOverlayPainter(this);
    m_busyWidget->setSequence(KIconLoader::global()->loadPixmapSequence("process-working", KIconLoader::SizeSmallMedium));
    m_busyWidget->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    m_busyWidget->setWidget(ui->busySpinner);
    ui->busySpinner->setFixedSize(24, 24);
    ui->busySpinner->show();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_busyWidget->start();

    IsoResult isoResult = verifyISO();

    ui->busySpinner->hide();
    m_busyWidget->stop();
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    delete m_busyWidget;
    
    if (isoResult.resultType == Invalid) {
        QMessageBox::critical(this, i18n("Invalid ISO"), i18n("ISO is invalid:<p>%1", isoResult.error));
        return;
    } else if (isoResult.resultType == DinnaeKen) {
        QMessageBox::StandardButton warningResult = QMessageBox::warning(this, i18n("Could not Verify ISO"), i18n("%1<p>Do you want to continue", isoResult.error), 
                                                                         QMessageBox::Yes|QMessageBox::No);
        if (warningResult == QMessageBox::No) {
            return;
        }
    }
    // Enable the Write button (if there are USB flash disks present)
    m_writeButton->setEnabled(ui->deviceList->count() > 0);
}

// Frees the GUI-specific allocated resources
void MainDialog::cleanup()
{
    // Delete all the formerly allocated UsbDevice objects attached to the combobox entries
    for (int i = 0; i < ui->deviceList->count(); ++i)
    {
        delete ui->deviceList->itemData(i).value<UsbDevice*>();
    }
}

// The reimplemented dragEnterEvent to inform which incoming drag&drop events are acceptable
void MainDialog::dragEnterEvent(QDragEnterEvent* event)
{
    // Accept only files with ANSI or Unicode paths (Windows) and URIs (Linux)
    if (event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileName\"") ||
        event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileNameW\"") ||
        event->mimeData()->hasFormat("text/uri-list"))
        event->accept();
}

// The reimplemented dropEvent to process the dropped file
void MainDialog::dropEvent(QDropEvent* event)
{
    QString newImageFile = "";
    QByteArray droppedFileName;

    // First, try to use the Unicode file name
    droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileNameW\"");
    if (!droppedFileName.isEmpty())
    {
        newImageFile = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(droppedFileName.constData()));
    }
    else
    {
        // If failed, use the ANSI name with the local codepage
        droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileName\"");
        if (!droppedFileName.isEmpty())
        {
            newImageFile = QString::fromLocal8Bit(droppedFileName.constData());
        }
        else
        {
            // And, finally, try the URI
            droppedFileName = event->mimeData()->data("text/uri-list");
            if (!droppedFileName.isEmpty())
            {
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
    if (newImageFile != "")
    {
        // If something was realy received update the information
        preprocessImageFile(newImageFile);
    }
}

// The reimplemented keyPressEvent to display confirmation if user closes the dialog during operation
void MainDialog::closeEvent(QCloseEvent* event)
{
    if (m_IsWriting)
    {
        if (QMessageBox::question(this, ApplicationTitle, i18n("Writing is in progress, abort it?")) == QMessageBox::No)
            event->ignore();
    }
}

// The reimplemented keyPressEvent to display confirmation if Esc is pressed during operation
// (it normally closes the dialog but does not issue closeEvent for unknown reason)
void MainDialog::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Escape) && m_IsWriting)
    {
        if (QMessageBox::question(this, ApplicationTitle, i18n("Writing is in progress, abort it?")) == QMessageBox::No)
            return;
    }
    QDialog::keyPressEvent(event);
}

// Suggests to select image file using the Open File dialog
void MainDialog::openImageFile()
{
    QString newImageFile = QFileDialog::getOpenFileName(this, "", m_LastOpenedDir, 
                                                        i18n("Disk Images (%1)", QString("*.iso *.bin *.img")) + ";;" + i18n("All Files (%1)", QString("(*)")),
                                                        NULL, QFileDialog::ReadOnly);
    if (newImageFile != "")
    {
        m_LastOpenedDir = newImageFile.left(newImageFile.lastIndexOf('/'));
        preprocessImageFile(newImageFile);
    }
}

// Schedules reloading the list of USB flash disks to run when possible
void MainDialog::scheduleEnumFlashDevices()
{
    if (m_IsWriting)
        m_EnumFlashDevicesWaiting = true;
    else
        enumFlashDevices();
}

void addFlashDeviceCallback(void* cbParam, UsbDevice* device)
{
    Ui::MainDialog* ui = (Ui::MainDialog*)cbParam;
    ui->deviceList->addItem(device->formatDisplayName(), QVariant::fromValue(device));
}

// Reloads the list of USB flash disks
void MainDialog::enumFlashDevices()
{
    // Remember the currently selected device
    QString selectedDevice = "";
    int idx = ui->deviceList->currentIndex();
    if (idx >= 0)
    {
        UsbDevice* dev = ui->deviceList->itemData(idx).value<UsbDevice*>();
        selectedDevice = dev->m_PhysicalDevice;
    }
    // Remove the existing entries
    cleanup();
    ui->deviceList->clear();
    // Disable the combobox
    // TODO: Disable the whole dialog
    ui->deviceList->setEnabled(false);

    platformEnumFlashDevices(addFlashDeviceCallback, ui);

    // Restore the previously selected device (if present)
    if (selectedDevice != "")
        for (int i = 0; i < ui->deviceList->count(); ++i)
        {
            UsbDevice* dev = ui->deviceList->itemData(i).value<UsbDevice*>();
            if (dev->m_PhysicalDevice == selectedDevice)
            {
                ui->deviceList->setCurrentIndex(i);
                break;
            }
        }
    // Reenable the combobox
    ui->deviceList->setEnabled(true);
    // Update the Write button enabled/disabled state
    m_writeButton->setEnabled((ui->deviceList->count() > 0) && (m_ImageFile != ""));
    // Update the Clear button enabled/disabled state
    m_clearButton->setEnabled(ui->deviceList->count() > 0);
}

// TODO currently separate classes for each distro, should be made 
// much more generic to avoid overhead and repetition
IsoResult MainDialog::verifyISO() {
    ui->verificationResultLabel->show();
    ui->verificationResultLabel->setText(i18n("Running ISO verification, please wait..."));

    QCoreApplication::instance()->processEvents();
    IsoResult result;
    VerifyNeonISO verifyNeon(m_ImageFile);
    if (verifyNeon.canVerify()) {
        if (verifyNeon.isValid()) {
            ui->verificationResultLabel->setText(i18n("Verified as valid KDE neon ISO"));
            result.resultType = Fine;
            result.error = i18n("Verified as valid KDE neon ISO");
            return result;
        } else {
            QString error(i18n("Invalid KDE neon image"));
            ui->verificationResultLabel->show();
            ui->verificationResultLabel->setText(verifyNeon.m_error);
            result.resultType = Invalid;
            result.error = verifyNeon.m_error;
            return result;
        }
    }
    VerifyArchISO verifyArch(m_ImageFile);
    if (verifyArch.canVerify()) {
        if (verifyArch.isValid()) {
            ui->verificationResultLabel->setText(i18n("Verified as valid Arch ISO"));
            result.resultType = Fine;
            result.error = i18n("Verified as valid Arch ISO");
            return result;
        } else {
            QString error(i18n("Invalid Arch image"));
            ui->verificationResultLabel->show();
            ui->verificationResultLabel->setText(verifyArch.m_error);
            result.resultType = Invalid;
            result.error = verifyArch.m_error;
            return result;
        }
    }
    VerifyNetrunnerISO verifyNetrunner(m_ImageFile);
    if (verifyNetrunner.canVerify()) {
        if (verifyNetrunner.isValid()) {
            ui->verificationResultLabel->setText(i18n("Verified as valid Netrunner ISO"));
            result.resultType = Fine;
            result.error = i18n("Verified as valid Netrunner ISO");
            return result;
        } else {
            QString error(i18n("Invalid Netrunner image"));
            ui->verificationResultLabel->setText(verifyNetrunner.m_error);
            result.resultType = Invalid;
            result.error = verifyNetrunner.m_error;
            return result;
        }
    }    
    VerifyKubuntuISO verifyKubuntu(m_ImageFile);
    if (verifyKubuntu.canVerify()) {
        if (verifyKubuntu.isValid()) {
            ui->verificationResultLabel->setText(i18n("Verified as valid Kubuntu ISO"));
            result.resultType = Fine;
            result.error = i18n("Verified as valid Kubuntu ISO");
            return result;
        } else {
            QString error(i18n("Invalid Kubuntu image"));
            ui->verificationResultLabel->show();
            ui->verificationResultLabel->setText(verifyKubuntu.m_error);
            result.resultType = Invalid;
            result.error = verifyKubuntu.m_error;
            return result;
        }
    }    
    QString error(i18n("Could not verify as a known distro image."));
    qDebug() << "verify error: " << error;
    ui->verificationResultLabel->setText(error);
    result.resultType = DinnaeKen;
    result.error = QString(i18n("Could not verify as a known distro image."));
    return result;
}

// Starts writing data to the device
void MainDialog::writeToDeviceKAuth(bool zeroing)
{
    qCDebug(ISOIMAGEWRITER_LOG) << "writeToDeviceKAuth()";
    if ((ui->deviceList->count() == 0) || (!zeroing && (m_ImageFile == "")))
        return;
    UsbDevice* selectedDevice = ui->deviceList->itemData(ui->deviceList->currentIndex()).value<UsbDevice*>();
    if (!zeroing && (m_ImageSize > selectedDevice->m_Size))
    {
        QLocale currentLocale;
        QMessageBox::critical(
            this,
            ApplicationTitle,
            i18n("The image is larger than your selected device!") + "\n" +
            i18n("Image size: %1MiB (%2b)", QString::number(m_ImageSize / DEFAULT_UNIT), currentLocale.toString(m_ImageSize)) + "\n" +
            i18n("Disk size: %1MiB (%2b)", QString::number(selectedDevice->m_Size / DEFAULT_UNIT), currentLocale.toString(selectedDevice->m_Size)),
            QMessageBox::Ok
        );
        return;
    }
    QMessageBox wipeWarningBox;
    wipeWarningBox.setText(i18n("All existing data on the selected device will be lost."));
    wipeWarningBox.setInformativeText(i18n("Are you sure you wish to proceed?"));
    wipeWarningBox.setIcon(QMessageBox::Warning);
    wipeWarningBox.addButton(QMessageBox::Ok);
    wipeWarningBox.addButton(QMessageBox::Cancel);
    wipeWarningBox.button(QMessageBox::Ok)->setText(i18n("Clear Disk and Write Image"));
    wipeWarningBox.exec();
    if (wipeWarningBox.result() != QMessageBox::Ok) {
        return;
    }

    connect(m_cancelButton, &QPushButton::clicked, this, &MainDialog::cancelWriting);
    KAuth::Action action(QLatin1String("org.kde.isoimagewriter.writefile"));
    action.setHelperId("org.kde.isoimagewriter");
    QVariantMap helperargs;
    //helperargs[QStringLiteral("filename")] = "bar";
    helperargs[QStringLiteral("zeroing")] = QVariant(zeroing);
    helperargs[QStringLiteral("imagefile")] = m_ImageFile;
    helperargs[QStringLiteral("usbdevice_visiblename")] = selectedDevice->m_VisibleName;
    helperargs[QStringLiteral("usbdevice_volumes")] = selectedDevice->m_Volumes[0];
    qCDebug(ISOIMAGEWRITER_LOG) << "volumes" << selectedDevice->m_Volumes[0];
    qCDebug(ISOIMAGEWRITER_LOG) << "size" << selectedDevice->m_Size;
    qCDebug(ISOIMAGEWRITER_LOG) << "m_SectorSize" << selectedDevice->m_SectorSize;
    helperargs[QStringLiteral("usbdevice_size")] = QString("%1").arg(selectedDevice->m_Size);
    helperargs[QStringLiteral("usbdevice_sectorsize")] = selectedDevice->m_SectorSize;
    helperargs[QStringLiteral("usbdevice_physicaldevice")] = selectedDevice->m_PhysicalDevice;

    action.setArguments(helperargs);
    action.setTimeout(3600000); // an hour
    m_job = action.execute();
    connect(m_job, SIGNAL(percent(KJob*, unsigned long)), this, SLOT(progressStep(KJob*, unsigned long)), Qt::DirectConnection);
    connect(m_job, SIGNAL(newData(const QVariantMap &)), this, SLOT(progressStep(const QVariantMap &)));
    connect(m_job, SIGNAL(statusChanged(KAuth::Action::AuthStatus)), this, SLOT(statusChanged(KAuth::Action::AuthStatus)));
    connect(m_job, SIGNAL(result(KJob*)), this, SLOT(finished(KJob*)));
    qCDebug(ISOIMAGEWRITER_LOG) << "runWriteImage start()";
    m_job->start();
    qCDebug(ISOIMAGEWRITER_LOG) << "action.isValid()? " << action.isValid();
    showWritingProgress(alignNumberDiv((zeroing ? DEFAULT_UNIT : m_ImageSize), DEFAULT_UNIT));
}

void MainDialog::cancelWriting() {
    qCDebug(ISOIMAGEWRITER_LOG) << "cancelWriting()";
    m_job->kill();
    qCDebug(ISOIMAGEWRITER_LOG) << "cancelWriting() done";
}

void MainDialog::progressStep(KJob* job, unsigned long step) {
    Q_UNUSED(job)
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep %() " << step;
    updateProgressBar(step);
}

void MainDialog::progressStep(const QVariantMap & data) {
    qCDebug(ISOIMAGEWRITER_LOG) << "progressStep(QVariantMap) ";// << step;
    if (data[QStringLiteral("progress")].isValid()) {
        int step = data[QStringLiteral("progress")].toInt();
        updateProgressBar(step);
    } else if (data[QStringLiteral("error")].isValid()) {
        showErrorMessage(data[QStringLiteral("error")].toString());
    } else if (data[QStringLiteral("success")].isValid()) {
        showSuccessMessage(data[QStringLiteral("success")].toString());
    }
}

void MainDialog::statusChanged(KAuth::Action::AuthStatus status) {
    qCDebug(ISOIMAGEWRITER_LOG) << "statusChanged: " << status;
}

void MainDialog::finished(KJob* job) {
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job->error();
    KAuth::ExecuteJob *job2 = (KAuth::ExecuteJob *)job;
    qCDebug(ISOIMAGEWRITER_LOG) << "finished() " << job2->data();
    hideWritingProgress();
}
     
// Starts writing the image
void MainDialog::writeImageToDevice()
{
    writeToDeviceKAuth(false);
}

// Clears the selected USB device
void MainDialog::clearDevice()
{
    writeToDeviceKAuth(true);
}

// Updates GUI to the "writing" mode (progress bar shown, controls disabled)
// Also sets the progress bar limits
void MainDialog::showWritingProgress(int maxValue)
{
    m_IsWriting = true;

    // Do not accept dropped files while writing
    setAcceptDrops(false);

    // Disable the main interface
    ui->imageLabel->setEnabled(false);
    ui->imageEdit->setEnabled(false);
    ui->imageSelectButton->setEnabled(false);
    ui->deviceLabel->setEnabled(false);
    ui->deviceList->setEnabled(false);
    ui->deviceRefreshButton->setEnabled(false);

    // Display and customize the progress bar part
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(maxValue);
    ui->progressBar->setValue(0);
    ui->progressBar->setVisible(true);
    ui->progressBarSpacer->changeSize(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_writeButton->setVisible(false);
    m_clearButton->setVisible(false);
    m_cancelButton->setVisible(true);

    // Expose the progress bar state to the OS
    m_ExtProgressBar.InitProgressBar(maxValue);
}

// Updates GUI to the "idle" mode (progress bar hidden, controls enabled)
void MainDialog::hideWritingProgress()
{
    m_IsWriting = false;

    // Reenable drag&drop
    setAcceptDrops(true);

    // Enable the main interface
    ui->imageLabel->setEnabled(true);
    ui->imageEdit->setEnabled(true);
    ui->imageSelectButton->setEnabled(true);
    ui->deviceLabel->setEnabled(true);
    ui->deviceList->setEnabled(true);
    ui->deviceRefreshButton->setEnabled(true);

    // Hide the progress bar and verification label
    ui->verificationResultLabel->hide();
    ui->progressBar->setVisible(false);
    ui->busySpinner->hide();
    ui->progressBarSpacer->changeSize(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_writeButton->setVisible(true);
    m_clearButton->setVisible(true);
    m_cancelButton->setVisible(false);

    // Send a signal that progressbar is no longer present
    m_ExtProgressBar.DestroyProgressBar();

    // If device list changed during writing update it now
    if (m_EnumFlashDevicesWaiting)
        enumFlashDevices();
}

// Increments the progress bar counter by the specified number
void MainDialog::updateProgressBar(int increment)
{
    int newValue = ui->progressBar->value() + increment;
    ui->progressBar->setValue(newValue);
    m_ExtProgressBar.SetProgressValue(newValue);
}

// Displays the message about successful completion and returns to the "idle" mode
void MainDialog::showSuccessMessage(QString msg)
{
    QMessageBox::information(
        this,
        ApplicationTitle,
        msg
    );
    hideWritingProgress();
}

// Displays the specified error message and returns to the "idle" mode
void MainDialog::showErrorMessage(QString msg)
{
    m_ExtProgressBar.ProgressSetError();
    QMessageBox::critical(
        this,
        ApplicationTitle,
        msg
    );
    hideWritingProgress();
}
