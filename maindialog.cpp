#include <Wbemidl.h>

#include <QMessageBox>
#include <QFileDialog>
#include <QDropEvent>
#include <QMimeData>
#include <QLocale>
#include <QThread>

#include "common.h"
#include "maindialog.h"
#include "ui_maindialog.h"
#include "imagewriter.h"

MainDialog::MainDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MainDialog),
    m_ImageFile(""),
    m_ImageSize(0)
{
    ui->setupUi(this);
    setFixedHeight(size().height());
    hideWritingProgress();
    // TODO: Show dialog disabled, print "please wait", enumerate devices, then update the list, enable the window
    enumFlashDevices();
}

// TODO: Automatically detect inserting/removing USB devices and update the list
// Code snippet:
//    DEV_BROADCAST_DEVICEINTERFACE cNotifyFilter;
//    ZeroMemory(&cNotifyFilter, sizeof(cNotifyFilter));
//    cNotifyFilter.dbcc_size        = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
//    cNotifyFilter.dbcc_devicetype  = DBT_DEVTYP_DEVICEINTERFACE;
//    cNotifyFilter.dbcc_classguid   = GUID_DEVINTERFACE_VOLUME;
//    m_hDevNotify = RegisterDeviceNotification(m_hAppWnd, &cNotifyFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
// m_hAppWnd is a handle of the window receiving WM_DEVICECHANGE.

MainDialog::~MainDialog()
{
    for (int i = 0; i < ui->deviceList->count(); ++i)
    {
        delete ui->deviceList->itemData(i).value<UsbDevice*>();
    }
    delete ui;
}

#define CHECK_OK(code, msg)         \
    {                               \
        HRESULT res = code;         \
        if (res != S_OK)            \
        {                           \
            wcscpy_s(err_msg, msg); \
            throw res;              \
        }                           \
    }

#define SAFE_RELEASE(obj)   \
    {                       \
        if (obj != NULL)    \
        {                   \
            obj->Release(); \
            obj = NULL;     \
        }                   \
    }

#define ALLOC_BSTR(name, str)                                               \
    BSTR name = SysAllocString(str);                                        \
    if (name == NULL)                                                       \
    {                                                                       \
        wcscpy_s(err_msg, L"Memory allocation for " ## L#name L" failed."); \
        throw (HRESULT)0;                                                   \
    }

#define FREE_BSTR(str)      \
    {                       \
        SysFreeString(str); \
        str = NULL;         \
    }

void MainDialog::enumFlashDevices()
{
    ui->deviceList->clear();
    ui->deviceList->setEnabled(false);

    int ret_value = 0;
    const size_t ERR_BUF_SZ = 1024;
    wchar_t err_msg[ERR_BUF_SZ];

    BSTR bstrNamespace      = NULL;
    BSTR strQL              = NULL;
    BSTR strQueryDisks      = NULL;
    BSTR strQueryPartitions = NULL;
    BSTR strQueryLetters    = NULL;

    IWbemLocator*         pIWbemLocator         = NULL;
    IWbemServices*        pWbemServices         = NULL;
    IEnumWbemClassObject* pEnumDisksObject      = NULL;
    IEnumWbemClassObject* pEnumPartitionsObject = NULL;
    IEnumWbemClassObject* pEnumLettersObject    = NULL;
    IWbemClassObject*     pDiskObject           = NULL;
    IWbemClassObject*     pPartitionObject      = NULL;
    IWbemClassObject*     pLetterObject         = NULL;

    UsbDevice* deviceData = NULL;

    try
    {
        ALLOC_BSTR(bstrNamespace, L"root\\cimv2");
        ALLOC_BSTR(strQL, L"WQL");
        ALLOC_BSTR(strQueryDisks, L"SELECT * FROM Win32_DiskDrive WHERE InterfaceType = \"USB\"");

        CHECK_OK(CoCreateInstance(CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pIWbemLocator), L"CoCreateInstance(WbemAdministrativeLocator) failed.");
        CHECK_OK(pIWbemLocator->ConnectServer(bstrNamespace,  NULL, NULL, NULL, 0, NULL, NULL, &pWbemServices), L"ConnectServer failed.");
        CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryDisks, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumDisksObject), L"Failed to query USB flash devices.");

        for (;;)
        {
            ULONG uReturned;
            pEnumDisksObject->Next(WBEM_INFINITE, 1, &pDiskObject, &uReturned);
            if (uReturned == 0)
                break;

            VARIANT val;
            UsbDevice* deviceData = new UsbDevice;

            if (pDiskObject->Get(L"Model", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_VisibleName = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }

            if (pDiskObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_PhysicalDevice = QString::fromWCharArray(val.bstrVal);
                }
                VariantClear(&val);
            }


            if (pDiskObject->Get(L"Size", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
            {
                if (val.vt == VT_BSTR)
                {
                    deviceData->m_Size = QString::fromWCharArray(val.bstrVal).toULongLong();
                }
                VariantClear(&val);
            }

            SAFE_RELEASE(pDiskObject);

            QString qstrQueryPartitions = "ASSOCIATORS OF {Win32_DiskDrive.DeviceID='" + deviceData->m_PhysicalDevice + "'} WHERE AssocClass = Win32_DiskDriveToDiskPartition";
            ALLOC_BSTR(strQueryPartitions, reinterpret_cast<const wchar_t*>(qstrQueryPartitions.utf16()));

            CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryPartitions, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumPartitionsObject), L"Failed to query list of partitions.");
            for (;;)
            {
                pEnumPartitionsObject->Next(WBEM_INFINITE, 1, &pPartitionObject, &uReturned);
                if (uReturned == 0)
                    break;

                QString qstrQueryLetters = "";
                if (pPartitionObject->Get(L"DeviceID", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                {
                    if (val.vt == VT_BSTR)
                    {
                        qstrQueryLetters = QString::fromWCharArray(val.bstrVal);
                    }
                    VariantClear(&val);
                }
                SAFE_RELEASE(pPartitionObject);

                if (qstrQueryLetters != "")
                {
                    qstrQueryLetters = "ASSOCIATORS OF {Win32_DiskPartition.DeviceID='" + qstrQueryLetters + "'} WHERE AssocClass = Win32_LogicalDiskToPartition";
                    ALLOC_BSTR(strQueryLetters, reinterpret_cast<const wchar_t*>(qstrQueryLetters.utf16()));

                    CHECK_OK(pWbemServices->ExecQuery(strQL, strQueryLetters, WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumLettersObject), L"Failed to query list of logical disks.");
                    for (;;)
                    {
                        pEnumLettersObject->Next(WBEM_INFINITE, 1, &pLetterObject, &uReturned);
                        if (uReturned == 0)
                            break;

                        if (pLetterObject->Get(L"Caption", 0, &val, 0, 0) == WBEM_S_NO_ERROR)
                        {
                            if (val.vt == VT_BSTR)
                            {
                                deviceData->m_Volumes << QString::fromWCharArray(val.bstrVal);
                            }
                            VariantClear(&val);
                        }
                        SAFE_RELEASE(pLetterObject);
                    }

                    SAFE_RELEASE(pEnumLettersObject);
                    FREE_BSTR(strQueryLetters);
                }
            }

            SAFE_RELEASE(pEnumPartitionsObject);
            FREE_BSTR(strQueryPartitions);

            QString displayName = ((deviceData->m_Volumes.size() == 0) ? "<unmounted>" : deviceData->m_Volumes.join(", ")) + " - " + deviceData->m_VisibleName + " (" + QString::number(alignNumberDiv(deviceData->m_Size, DEFAULT_UNIT)) + " MB)";
            ui->deviceList->addItem(displayName, QVariant::fromValue(deviceData));
            deviceData = NULL;
        }
    }
    catch (HRESULT err_code)
    {
        wprintf(L"Error: %s", err_msg);
        if (err_code != 0)
            wprintf(L" (Code: 0x%08lx)", err_code);
        wprintf(L"\n");
        ret_value = 1;
    }

    delete deviceData;

    SAFE_RELEASE(pLetterObject);
    SAFE_RELEASE(pPartitionObject);
    SAFE_RELEASE(pDiskObject);
    SAFE_RELEASE(pEnumDisksObject);
    SAFE_RELEASE(pEnumPartitionsObject);
    SAFE_RELEASE(pEnumLettersObject);
    SAFE_RELEASE(pWbemServices);
    SAFE_RELEASE(pIWbemLocator);

    FREE_BSTR(bstrNamespace);
    FREE_BSTR(strQL);
    FREE_BSTR(strQueryDisks);
    FREE_BSTR(strQueryPartitions);
    FREE_BSTR(strQueryLetters);

    ui->deviceList->setEnabled(true);
}

void MainDialog::preprocessImageFile(const QString& newImageFile)
{
    m_ImageFile = newImageFile;
    m_ImageSize = 0;
    QString displayName = m_ImageFile;
    QFile f(m_ImageFile);
    if (f.open(QIODevice::ReadOnly))
    {
        m_ImageSize = f.size();
        displayName += " (" + QString::number(alignNumberDiv(m_ImageSize, DEFAULT_UNIT)) + " MB)";
        f.close();
    }
    ui->imageEdit->setText(displayName);
    ui->writeButton->setEnabled(true);
}

void MainDialog::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileName\"") ||
        event->mimeData()->hasFormat("application/x-qt-windows-mime;value=\"FileNameW\""))
        event->accept();
}

void MainDialog::dropEvent(QDropEvent* event)
{
    QString newImageFile = "";
    QByteArray droppedFileName;
    droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileNameW\"");
    if (!droppedFileName.isEmpty())
    {
        newImageFile = QString::fromWCharArray(reinterpret_cast<const wchar_t*>(droppedFileName.constData()));
    }
    else
    {
        droppedFileName = event->mimeData()->data("application/x-qt-windows-mime;value=\"FileName\"");
        if (!droppedFileName.isEmpty())
        {
            newImageFile = QString::fromLocal8Bit(droppedFileName.constData());
        }
    }
    if (newImageFile != "")
    {
        preprocessImageFile(newImageFile);
    }
}

void MainDialog::showWritingProgress()
{
    // Do not accept drag&drop while writing
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
}

void MainDialog::hideWritingProgress()
{
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
}

void MainDialog::openImageFile()
{
    QString newImageFile = QFileDialog::getOpenFileName(this, "", "", "Disk Images (*.iso;*.bin;*.img);;All Files(*.*)", NULL, QFileDialog::ReadOnly);
    if (newImageFile != "")
    {
        newImageFile.replace('/', '\\');
        preprocessImageFile(newImageFile);
    }
}

void MainDialog::writeImageToDevice()
{
    QLocale currentLocale;
    UsbDevice* selectedDevice = ui->deviceList->itemData(ui->deviceList->currentIndex()).value<UsbDevice*>();
    if (m_ImageSize > selectedDevice->m_Size)
    {
        QMessageBox::critical(
            this,
            ApplicationTitle,
            "The image is larger than your selected device!\nImage size: " + currentLocale.toString(m_ImageSize) + " bytes\nDisk size: " + currentLocale.toString(selectedDevice->m_Size) + " bytes",
            QMessageBox::Ok
        );
        return;
    }
    if (QMessageBox::warning(
            this,
            ApplicationTitle,
            "Writing an image will erase all existing data on the selected device.\nAre you sure you wish to proceed?",
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

    // If error is sent from the worker display it in the current GUI thread
    connect(writer, &ImageWriter::error, this, &MainDialog::showErrorMessage);

    // Restore the normal view of the dialog when writing is finished
    // TODO: If a messagebox is displayed, wait until it's closed, and only then restore the dialog!
    connect(writer, &ImageWriter::finished, this, &MainDialog::hideWritingProgress);

    // Now start the writer thread
    writer->moveToThread(writerThread);
    writerThread->start();
}

void MainDialog::updateProgressBar(int increment)
{
    ui->progressBar->setValue(ui->progressBar->value() + increment);
}

void MainDialog::showErrorMessage(QString msg)
{
    QMessageBox::critical(
        this,
        ApplicationTitle,
        msg,
        QMessageBox::Ok
    );
}
