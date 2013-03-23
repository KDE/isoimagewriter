////////////////////////////////////////////////////////////////////////////////
// Implementation of MainDialog


#include <QMessageBox>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QLocale>
#include <QThread>
#include <QDir>
#include <QRegularExpression>

#include "common.h"
#include "maindialog.h"
#include "ui_maindialog.h"
#include "imagewriter.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog),
    m_ImageFile(""),
    m_ImageSize(0),
    m_LastOpenedDir(""),
    m_IsWriting(false),
    m_EnumFlashDevicesWaiting(false)
#ifdef Q_OS_WIN32
    , m_Win7TaskbarList(NULL)
#endif
{
    ui->setupUi(this);
    // Remove the Context Help button and add the Minimize button to the titlebar
    setWindowFlags((windowFlags() | Qt::CustomizeWindowHint | Qt::WindowMinimizeButtonHint) & ~Qt::WindowContextHelpButtonHint);
    // Disallow to change the dialog height
    setFixedHeight(size().height());
    // Start in the "idle" mode
    hideWritingProgress();
    // Load the list of USB flash disks
    enumFlashDevices();
    // TODO: Increase the dialog display speed by showing it with the empty list and enumerating devices
    // in the background (dialog disabled, print "please wait")
    // TODO: Use dialog disabling also for manual refreshing the list
    // TODO: Automatically detect inserting/removing USB devices and update the list

#ifdef Q_OS_WIN32
    // Get the taskbar object (if NULL is returned it won't be used)
    CoCreateInstance(CLSID_TaskbarList, NULL, CLSCTX_ALL, IID_ITaskbarList3, reinterpret_cast<void**>(&m_Win7TaskbarList));
#endif

    // When device changing event comes, refresh the list of USB flash disks
    // Using QueuedConnection to avoid delays in processing the message
    connect(this, &MainDialog::deviceChanged, this, &MainDialog::scheduleEnumFlashDevices, Qt::QueuedConnection);
}

MainDialog::~MainDialog()
{
#ifdef Q_OS_WIN32
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->Release();
#endif
    cleanup();
    delete ui;
}

// Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages
bool MainDialog::nativeEventFilter(const QByteArray& /*eventType*/, void* message, long* result)
{
#ifdef Q_OS_WIN32
    MSG* msg = static_cast<MSG*>(message);
    if ((msg->message == WM_DEVICECHANGE) &&
        ((msg->wParam == DBT_DEVICEARRIVAL) || (msg->wParam == DBT_DEVICEREMOVECOMPLETE)))
    {
        // If the event was caused by adding or remiving a device, mark the WinAPI message as processed
        // and emit the notification signal
        *result = TRUE;
        emit deviceChanged();
        return true;
    }
#endif
    return false;
}

// Retrieves information about the selected file and displays it in the dialog
void MainDialog::preprocessImageFile(const QString& newImageFile)
{
    m_ImageFile = newImageFile;
    m_ImageSize = 0;
    QString displayName = QDir::toNativeSeparators(m_ImageFile);
    QFile f(m_ImageFile);
    if (f.open(QIODevice::ReadOnly))
    {
        m_ImageSize = f.size();
        displayName += " (" + QString::number(alignNumberDiv(m_ImageSize, DEFAULT_UNIT)) + " " + tr("MB") + ")";
        f.close();
    }
    ui->imageEdit->setText(displayName);
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
    // Accept only files with ANSI or Unicode paths
    if (event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileName\"") ||
        event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileNameW\""))
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
        if (QMessageBox::question(this, ApplicationTitle, tr("Writing is in progress, abort it?")) == QMessageBox::No)
            event->ignore();
    }
}

// The reimplemented keyPressEvent to display confirmation if Esc is pressed during operation
// (it normally closes the dialog but does not issue closeEvent for unknown reason)
void MainDialog::keyPressEvent(QKeyEvent* event)
{
    if ((event->key() == Qt::Key_Escape) && m_IsWriting)
    {
        if (QMessageBox::question(this, ApplicationTitle, tr("Writing is in progress, abort it?")) == QMessageBox::No)
            return;
    }
    QDialog::keyPressEvent(event);
}

// Suggests to select image file using the Open File dialog
void MainDialog::openImageFile()
{
    QString newImageFile = QFileDialog::getOpenFileName(this, "", m_LastOpenedDir, tr("Disk Images") + " (*.iso *.bin *.img);;" + tr("All Files") + " (*.*)", NULL, QFileDialog::ReadOnly);
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

// Reloads the list of USB flash disks
void MainDialog::enumFlashDevices()
{
    // Remove the existing entries
    cleanup();
    ui->deviceList->clear();
    // Disable the combobox
    // TODO: Disable the whole dialog
    ui->deviceList->setEnabled(false);

#if defined(Q_OS_LINUX)
    // Using /sys/bus/usb/devices directory contents for enumerating the USB devices
    //
    // Details:
    // Take the devices which have <device>/bInterfaceClass contents set to "08" (storage device).
    //
    // 1. To get the user-friendly name we need to read the <manufacturer> and <product> files
    //    of the parent device (the parent device is the one with less-specified name, e.g. "2-1" for "2-1:1.0").
    //
    // 2. The block device name can be found by searching the contents of the following subdirectory:
    //      <device>/host*/target*/<scsi-device-name>/block/
    //    where * is a placeholder, and <scsi-device-name> starts with the same substring that "target*" ends with.
    //    For example, this path may look like follows:
    //      /sys/bus/usb/devices/1-1:1.0/host4/target4:0:0/4:0:0:0/block/
    //    This path contains the list of block devices by their names, e.g. sdc, which gives us /dev/sdc.
    //
    // 3. And, finally, for the device size we multiply .../block/sdX/size (the number of sectors) with
    //    .../block/sdX/queue/logical_block_size (the sector size).

    // Start with enumerating all the USB devices
    QString usbDevicesRoot = "/sys/bus/usb/devices";
    QDir dirList(usbDevicesRoot);
    QStringList usbDevices = dirList.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < usbDevices.size(); ++i)
    {
        QDir subdirList = dirList;
        if (!subdirList.cd(usbDevices[i]))
            continue;

        // Skip devices with wrong interface class
        if (readFileContents(subdirList.absoluteFilePath("bInterfaceClass")) != "08\n")
            continue;

        // Search for "host*" entries and take the first one
        // TODO: Find out whether several host entries may appear
        QStringList hosts = subdirList.entryList(QStringList("host*"));
        if (hosts.size() == 0)
            continue;
        if (!subdirList.cd(hosts[0]))
            continue;

        // Search for "target*" entries and take the first one
        // TODO: Find out whether several target entries may appear
        QStringList targets = subdirList.entryList(QStringList("target*"));
        if (targets.size() == 0)
            continue;
        if (!subdirList.cd(targets[0]))
            continue;

        // Remove the "target" part and append "*" to search for appropriate SCSI devices
        // Again, take the first one
        // TODO: Find out whether several SCSI devices may appear
        QStringList scsiTargets = subdirList.entryList(QStringList(targets[0].mid(6) + "*"));
        if (scsiTargets.size() == 0)
            continue;

        // Read the list of block devices and take the first one
        // TODO: Find out whether several block devices may appear
        // TODO: Check what happens with card readers with several cards inserted
        // Preliminary googling showed that several SCSI LUNs may appear in this case
        if (!subdirList.cd(scsiTargets[0] + "/block"))
            continue;
        QStringList blockDevices = subdirList.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        if (blockDevices.size() == 0)
            continue;

        // Store the necessary information in the UsbDevice object
        UsbDevice* deviceData = new UsbDevice;

        // Use the block device name as both physical device and displayed volume name
        deviceData->m_PhysicalDevice = "/dev/" + blockDevices[0];
        deviceData->m_Volumes << deviceData->m_PhysicalDevice;

        // Get the device size
        quint64 blocksNum = readFileContents(subdirList.absoluteFilePath(blockDevices[0] + "/size")).toULongLong();
        // TODO: Find out whether size is counted in logical or physical blocks
        uint blockSize = readFileContents(subdirList.absoluteFilePath(blockDevices[0] + "/queue/logical_block_size")).toUInt();
        if (blockSize == 0)
            blockSize = 512;
        deviceData->m_Size = blocksNum * blockSize;

        // Get the user-friendly name for the device by reading the parent device fields
        QString usbParentDevice = usbDevices[i];
        usbParentDevice.replace(QRegularExpression("^(\\d+-\\d+):.*$"), "\\1");
        usbParentDevice.prepend(usbDevicesRoot + "/");
        QString manufacturer = readFileContents(usbParentDevice + "/manufacturer").trimmed();
        QString product = readFileContents(usbParentDevice + "/product").trimmed();
        if ((manufacturer != "") || (product != ""))
        {
            // If only one of the <manufacturer> and <product> is non-empty, make it the device name,
            // otherwise concatenate via space character
            // TODO: Find out how to get more "friendly" name (for SATA-USB connector it shows the bridge
            // device name instead of the disk drive name)
            deviceData->m_VisibleName = (manufacturer + " " + product).trimmed();
        }

        // The device information is now complete, construct the display name for the combobox and append the entry
        // Format is: "<volume(s)> - <user-friendly name> (<size in megabytes>)"
        QString displayName = ((deviceData->m_Volumes.size() == 0) ? tr("<unmounted>") : deviceData->m_Volumes.join(", ")) + " - " + deviceData->m_VisibleName + " (" + QString::number(alignNumberDiv(deviceData->m_Size, DEFAULT_UNIT)) + " " + tr("MB") + ")";
        ui->deviceList->addItem(displayName, QVariant::fromValue(deviceData));
        // The object is now under the combobox control, nullify the pointer
        deviceData = NULL;
    }

#elif defined(Q_OS_WIN32)
    // Using WMI for enumerating the USB devices

    // Namespace of the WMI classes
    BSTR strNamespace       = NULL;
    // "WQL" - the query language we're gonna use (the only possible, actually)
    BSTR strQL              = NULL;
    // Query string for requesting physical devices
    BSTR strQueryDisks      = NULL;
    // Query string for requesting partitions for each of the the physical devices
    BSTR strQueryPartitions = NULL;
    // Query string for requesting logical disks for each of the partitions
    BSTR strQueryLetters    = NULL;

    // Various COM objects for executing the queries, enumerating lists and retrieving properties
    IWbemLocator*         pIWbemLocator         = NULL;
    IWbemServices*        pWbemServices         = NULL;
    IEnumWbemClassObject* pEnumDisksObject      = NULL;
    IEnumWbemClassObject* pEnumPartitionsObject = NULL;
    IEnumWbemClassObject* pEnumLettersObject    = NULL;
    IWbemClassObject*     pDiskObject           = NULL;
    IWbemClassObject*     pPartitionObject      = NULL;
    IWbemClassObject*     pLetterObject         = NULL;

    // Temporary object for attaching data to the combobox entries
    UsbDevice* deviceData = NULL;

    try
    {
        // Start with allocating the fixed strings
        ALLOC_BSTR(strNamespace, L"root\\cimv2");
        ALLOC_BSTR(strQL, L"WQL");
        ALLOC_BSTR(strQueryDisks, L"SELECT * FROM Win32_DiskDrive WHERE InterfaceType = \"USB\"");

        // Create the IWbemLocator and execute the first query (list of physical disks attached via USB)
        CHECK_OK(CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, reinterpret_cast<void**>(&pIWbemLocator)), tr("CoCreateInstance(WbemAdministrativeLocator) failed."));
        CHECK_OK(pIWbemLocator->ConnectServer(strNamespace,  NULL, NULL, NULL, 0, NULL, NULL, &pWbemServices), tr("ConnectServer failed."));
        CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryDisks, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDisksObject), tr("Failed to query USB flash devices."));

        // Enumerate the received list of devices
        for (;;)
        {
            // Get the next available device or exit the loop
            ULONG uReturned;
            pEnumDisksObject->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturned);
            if (uReturned == 0)
                break;

            VARIANT val;

            // Fetch the required properties and store them in the UsbDevice object
            UsbDevice* deviceData = new UsbDevice;

            // User-friendly name of the device
            if (pDiskObject->Get(L"Model", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_VisibleName = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }

            // System name of the device
            if (pDiskObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_PhysicalDevice = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }

            // Size of the devifce
            if (pDiskObject->Get(L"Size", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_Size = QString::fromWCharArray(val.bstrVal).toULongLong();
                }
                VariantClear(&val);
            }

            // The device object is no longer needed, release it
            SAFE_RELEASE(pDiskObject);

            // Construct the request for listing the partitions on the current disk
            QString qstrQueryPartitions = "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='" + deviceData->m_PhysicalDevice + "'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
            ALLOC_BSTR(strQueryPartitions, reinterpret_cast<const wchar_t*>(qstrQueryPartitions.utf16()));

            // Execute the query
            CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryPartitions, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumPartitionsObject), tr("Failed to query list of partitions."));

            // Enumerate the received list of partitions
            for (;;)
            {
                // Get the next available partition or exit the loop
                pEnumPartitionsObject->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturned);
                if (uReturned == 0)
                    break;

                // Fetch the DeviceID property and store it for using in the next request
                QString qstrQueryLetters = "";
                if (pPartitionObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                {
                    if (val.vt == VT_BSTR)
                    {
                        qstrQueryLetters = QString::fromWCharArray(val.bstrVal);
                    }
                    VariantClear(&val);
                }

                // The partition object is no longer needed, release it
                SAFE_RELEASE(pPartitionObject);

                // If DeviceID was fetched proceed to the logical disks
                if (qstrQueryLetters != "")
                {
                    // Construct the request for listing the logical disks related to the current partition
                    qstrQueryLetters = "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" + qstrQueryLetters + "'} WHERE AssocClass = Win32_LogicalDiskToPartition";
                    ALLOC_BSTR(strQueryLetters, reinterpret_cast<const wchar_t*>(qstrQueryLetters.utf16()));

                    // Execute the query
                    CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryLetters, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumLettersObject), tr("Failed to query list of logical disks."));

                    // Enumerate the received list of logical disks
                    for (;;)
                    {
                        // Get the next available logical disk or exit the loop
                        pEnumLettersObject->Next(WBEM_INFINITE, 1, &pLetterObject, &uReturned);
                        if (uReturned == 0)
                            break;

                        // Fetch the disk letter and add it to the list of volumes in the UsbDevice object
                        if (pLetterObject->Get(L"Caption", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                        {
                            if (val.vt == VT_BSTR)
                            {
                                deviceData->m_Volumes << QString::fromWCharArray(val.bstrVal);
                            }
                            VariantClear(&val);
                        }

                        // The logical disk object is no longer needed, release it
                        SAFE_RELEASE(pLetterObject);
                    }

                    // Release the logical disks enumerator object and the corresponding query string
                    SAFE_RELEASE(pEnumLettersObject);
                    FREE_BSTR(strQueryLetters);
                }
            }

            // Release the partitions enumerator object and the corresponding query string
            SAFE_RELEASE(pEnumPartitionsObject);
            FREE_BSTR(strQueryPartitions);

            // The device information is now complete, construct the display name for the combobox and append the entry
            // Format is: "<volume(s)> - <user-friendly name> (<size in megabytes>)"
            QString displayName = ((deviceData->m_Volumes.size() == 0) ? tr("<unmounted>") : deviceData->m_Volumes.join(", ")) + " - " + deviceData->m_VisibleName + " (" + QString::number(alignNumberDiv(deviceData->m_Size, DEFAULT_UNIT)) + " " + tr("MB") + ")";
            ui->deviceList->addItem(displayName, QVariant::fromValue(deviceData));
            // The object is now under the combobox control, nullify the pointer
            deviceData = NULL;
        }
    }
    catch (QString errMessage)
    {
        // Something bad happened
        QMessageBox::critical(
            this,
            ApplicationTitle,
            errMessage
        );
    }

    // The cleanup stage
    if (deviceData != NULL)
        delete deviceData;

    SAFE_RELEASE(pLetterObject);
    SAFE_RELEASE(pPartitionObject);
    SAFE_RELEASE(pDiskObject);
    SAFE_RELEASE(pEnumDisksObject);
    SAFE_RELEASE(pEnumPartitionsObject);
    SAFE_RELEASE(pEnumLettersObject);
    SAFE_RELEASE(pWbemServices);
    SAFE_RELEASE(pIWbemLocator);

    FREE_BSTR(strNamespace);
    FREE_BSTR(strQL);
    FREE_BSTR(strQueryDisks);
    FREE_BSTR(strQueryPartitions);
    FREE_BSTR(strQueryLetters);
#endif

    // Reenable the combobox
    ui->deviceList->setEnabled(true);
    // Update the Write button enabled/disabled state
    ui->writeButton->setEnabled((ui->deviceList->count() > 0) && (m_ImageFile != ""));
}

// Starts writing the image
void MainDialog::writeImageToDevice()
{
    QLocale currentLocale;
    if ((ui->deviceList->count() == 0) || (m_ImageFile == ""))
        return;
    UsbDevice* selectedDevice = ui->deviceList->itemData(ui->deviceList->currentIndex()).value<UsbDevice*>();
    if (m_ImageSize > selectedDevice->m_Size)
    {
        QMessageBox::critical(
            this,
            ApplicationTitle,
            tr("The image is larger than your selected device!") + "\n" +
            tr("Image size:") + " " + QString::number(m_ImageSize / DEFAULT_UNIT) + " " + tr("MB") + " (" + currentLocale.toString(m_ImageSize) + " " + tr("b") + ")\n" +
            tr("Disk size:") + " " + QString::number(selectedDevice->m_Size / DEFAULT_UNIT) + " " + tr("MB") + " (" + currentLocale.toString(selectedDevice->m_Size) + " " + tr("b") + ")",
            QMessageBox::Ok
        );
        return;
    }
    if (QMessageBox::warning(
            this,
            ApplicationTitle,
            tr("Writing an image will erase all existing data on the selected device.\n"
               "Are you sure you wish to proceed?"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No) == QMessageBox::No)
        return;

    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(alignNumberDiv(m_ImageSize, DEFAULT_UNIT));
    ui->progressBar->setValue(0);
    showWritingProgress();

    ImageWriter* writer = new ImageWriter(m_ImageFile, selectedDevice);
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

// Updates GUI to the "writing" mode (progress bar shown, controls disabled)
void MainDialog::showWritingProgress()
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
    ui->progressBar->setVisible(true);
    ui->progressBarSpacer->changeSize(0, 10, QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->writeButton->setVisible(false);
    ui->cancelButton->setVisible(true);

#ifdef Q_OS_WIN32
    // Add the progress indicator to the taskbar button
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->SetProgressValue(reinterpret_cast<HWND>(winId()), 0, ui->progressBar->maximum());
#endif
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
    ui->cancelButton->setVisible(false);

#ifdef Q_OS_WIN32
    // Remove progress indicator from the taskbar button
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->SetProgressState(reinterpret_cast<HWND>(winId()), TBPF_NOPROGRESS);
#endif

    // If device list changed during writing update it now
    if (m_EnumFlashDevicesWaiting)
        enumFlashDevices();
}

// Increments the progress bar counter by the specified number
void MainDialog::updateProgressBar(int increment)
{
    int newValue = ui->progressBar->value() + increment;
    ui->progressBar->setValue(newValue);
#ifdef Q_OS_WIN32
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->SetProgressValue((HWND)this->winId(), newValue, ui->progressBar->maximum());
#endif
}

// Displays the message about successful completion and returns to the "idle" mode
void MainDialog::showSuccessMessage()
{
    QMessageBox::information(
        this,
        ApplicationTitle,
        tr("The operation completed successfully.")
    );
    hideWritingProgress();
}

// Displays the specified error message and returns to the "idle" mode
void MainDialog::showErrorMessage(QString msg)
{
#ifdef Q_OS_WIN32
    if (m_Win7TaskbarList != NULL)
        m_Win7TaskbarList->SetProgressState(reinterpret_cast<HWND>(winId()), TBPF_ERROR);
#endif
    QMessageBox::critical(
        this,
        ApplicationTitle,
        msg
    );
    hideWritingProgress();
}
