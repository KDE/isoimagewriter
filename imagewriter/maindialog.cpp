/*
 * Copyright 2016 ROSA
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

#include "common.h"
#include "mainapplication.h"
#include "maindialog.h"
#include "ui_maindialog.h"
#include "imagewriter.h"
#include "usbdevice.h"

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
    QLoggingCategory::setFilterRules(QStringLiteral("org.kde.imagewriter = true"));

    ui->setupUi(this);

#if defined(ROSA_BRANDING)
    // Compile with -DROSA_BRANDING=On to use the ROSA name
    setWindowTitle("ROSA Image Writer");
    ui->logo->setPixmap(QPixmap(QStandardPaths::locate(QStandardPaths::AppDataLocation, "logo-rosa.png")));
#endif
    ui->imageSelectButton->setIcon(QIcon::fromTheme("folder-open"));
    ui->deviceRefreshButton->setIcon(QIcon::fromTheme("view-refresh"));
    // Remove the Context Help button and add the Minimize button to the titlebar
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint) & ~Qt::WindowContextHelpButtonHint);
    // Disallow to change the dialog height
    setFixedHeight(size().height());
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
    ui->imageEdit->setText(QDir::toNativeSeparators(m_ImageFile) + i18n("(%1 MB)", QString::number(alignNumberDiv(m_ImageSize, DEFAULT_UNIT))));
    // Enable the Write button (if there are USB flash disks present)
    ui->writeButton->setEnabled(ui->deviceList->count() > 0);
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
    ui->writeButton->setEnabled((ui->deviceList->count() > 0) && (m_ImageFile != ""));
    // Update the Clear button enabled/disabled state
    ui->clearButton->setEnabled(ui->deviceList->count() > 0);
}

// Starts writing data to the device
void MainDialog::writeToDevice(bool zeroing)
{
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
            i18n("Image size: %1MB (%2b)", QString::number(m_ImageSize / DEFAULT_UNIT), currentLocale.toString(m_ImageSize)) + "\n" +
            i18n("Disk size: %1MB (%2b)", QString::number(selectedDevice->m_Size / DEFAULT_UNIT), currentLocale.toString(selectedDevice->m_Size)),
            QMessageBox::Ok
        );
        return;
    }
    if (QMessageBox::warning(
            this,
            ApplicationTitle,
            "<font color=\"red\">" + i18n("Warning!") + "</font> " + i18n("All existing data on the selected device will be lost!") + "<br>" +
            i18n("Are you sure you wish to proceed?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) == QMessageBox::No)
        return;

    showWritingProgress(alignNumberDiv((zeroing ? DEFAULT_UNIT : m_ImageSize), DEFAULT_UNIT));

    ImageWriter* writer = new ImageWriter(zeroing ? "" : m_ImageFile, selectedDevice);
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
    connect(ui->cancelButton, &QPushButton::clicked, writer, &ImageWriter::cancelWriting, Qt::DirectConnection);

    // Each time a block is written, update the progress bar
    connect(writer, &ImageWriter::blockWritten, this, &MainDialog::updateProgressBar);

    // Show the message about successful completion on success
    connect(writer, &ImageWriter::success, this, &MainDialog::showSuccessMessage);

    // Show error message if error is sent by the worker
    connect(writer, &ImageWriter::error, this, &MainDialog::showErrorMessage);

    // Silently return back to normal dialog form if the operation was cancelled
    connect(writer, &ImageWriter::cancelled, this, &MainDialog::hideWritingProgress);

    // Now start the writer thread
    writer->moveToThread(writerThread);
    writerThread->start();
}

// Starts writing the image
void MainDialog::writeImageToDevice()
{
    writeToDevice(false);
}

// Clears the selected USB device
void MainDialog::clearDevice()
{
    writeToDevice(true);
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
    ui->writeButton->setVisible(false);
    ui->clearButton->setVisible(false);
    ui->cancelButton->setVisible(true);

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

    // Hide the progress bar
    ui->progressBar->setVisible(false);
    ui->progressBarSpacer->changeSize(10, 10, QSizePolicy::Expanding, QSizePolicy::Fixed);
    ui->writeButton->setVisible(true);
    ui->clearButton->setVisible(true);
    ui->cancelButton->setVisible(false);

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
