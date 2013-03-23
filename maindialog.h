#ifndef MAINDIALOG_H
#define MAINDIALOG_H

////////////////////////////////////////////////////////////////////////////////
// MainDialog is the main application window


#include <QDialog>
#include <QAbstractNativeEventFilter>

#include "common.h"
#include "externalprogressbar.h"

namespace Ui {
    class MainDialog;
}

class MainDialog : public QDialog, public QAbstractNativeEventFilter
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

    // Implements QAbstractNativeEventFilter interface for processing WM_DEVICECHANGE messages
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result);

private:
    Ui::MainDialog *ui;

protected:
    // Image file currently selected by the user
    QString m_ImageFile;
    // Size of the image file (cached here to avoid excessive file system requests)
    quint64 m_ImageSize;
    // Remember the last opened directory to suggest it automatically on next Open
    QString m_LastOpenedDir;
    // Whether image is being written at the moment or not
    bool    m_IsWriting;
    // Flag indicating that flash disks enumerating is pending
    bool    m_EnumFlashDevicesWaiting;

    // Abstraction layer for projecting the progress bar into operating system (if supported)
    ExternalProgressBar m_ExtProgressBar;

    // Retrieves information about the selected file and displays it in the dialog
    void preprocessImageFile(const QString& newImageFile);
    // Frees the GUI-specific allocated resources
    void cleanup();

    // Reimplemented event handlers for drag&drop support
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    // Reimplemented event handlers for protecting dialog closing during operation
    void closeEvent(QCloseEvent* event);
    void keyPressEvent(QKeyEvent* event);

    // Reloads the list of USB flash disks
    void enumFlashDevices();

signals:
    // Emitted when device change notification arrives
    void deviceChanged();

public slots:
    // Suggests to select image file using the Open File dialog
    void openImageFile();
    // Schedules reloading the list of USB flash disks to run when possible
    void scheduleEnumFlashDevices();
    // Starts writing the image
    void writeImageToDevice();

    // Updates GUI to the "writing" mode (progress bar shown, controls disabled)
    // Also sets the progress bar limits
    void showWritingProgress(int maxValue);
    // Updates GUI to the "idle" mode (progress bar hidden, controls enabled)
    void hideWritingProgress();
    // Increments the progress bar counter by the specified number
    void updateProgressBar(int increment);
    // Displays the message about successful completion and returns to the "idle" mode
    void showSuccessMessage();
    // Displays the specified error message and returns to the "idle" mode
    void showErrorMessage(QString msg);
};


// Structure for attaching to the combobox entries of USB flash disks
class UsbDevice
{
public:
    UsbDevice() : m_VisibleName(QObject::tr("Unknown Device")), m_Volumes(), m_Size(0), m_PhysicalDevice("") {}

    // User-friendly name of the device
    QString     m_VisibleName;
    // List of mounted volumes from this device
    QStringList m_Volumes;
    // Size of the device
    quint64     m_Size;
    // System name of the physical disk
    QString     m_PhysicalDevice;
};

Q_DECLARE_METATYPE(UsbDevice*)


#ifdef Q_OS_WIN32
// Several WinAPI COM specific macros for keeping the code clean

// Runs the COM request specified, checks for return value and throws an exception
// with descriptive error message if it's not OK
#define CHECK_OK(code, msg)                       \
    {                                             \
        HRESULT res = code;                       \
        if (res != S_OK)                          \
        {                                         \
            throw errorMessageFromCode(msg, res); \
        }                                         \
    }

// Releases the COM object and nullifies the pointer
#define SAFE_RELEASE(obj)   \
    {                       \
        if (obj != NULL)    \
        {                   \
            obj->Release(); \
            obj = NULL;     \
        }                   \
    }

// Allocated a BSTR string using the specified text, checks for successful memory allocation
// and throws an exception with descriptive error message if unsuccessful
#define ALLOC_BSTR(name, str)                                        \
    {                                                                \
        name = SysAllocString(str);                                  \
        if (name == NULL)                                            \
        {                                                            \
            throw tr("Memory allocation for %1 failed.").arg(#name); \
        }                                                            \
    }

// Releases the BSTR string and nullifies the pointer
#define FREE_BSTR(str)      \
    {                       \
        SysFreeString(str); \
        str = NULL;         \
    }
#endif


#endif // MAINDIALOG_H
