#ifndef MAINDIALOG_H
#define MAINDIALOG_H

////////////////////////////////////////////////////////////////////////////////
// MainDialog is the main application window


#include <QDialog>

namespace Ui {
    class MainDialog;
}

class MainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();
    
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


public slots:
    // Suggests to select image file using the Open File dialog
    void openImageFile();
    // Reloads the list of USB flash disks
    void enumFlashDevices();
    // Starts writing the image
    void writeImageToDevice();

    // Updates GUI to the "writing" mode (progress bar shown, controls disabled)
    void showWritingProgress();
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
    UsbDevice() : m_VisibleName("Unknown Device"), m_Volumes(), m_Size(0), m_PhysicalDevice("") {}

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


// Several WinAPI COM specific macros for keeping the code clean
// TODO: Invent something clever to avoid fixed name of the error message buffer

// Runs the COM request specified, checks for return value and throws an exception if it's not OK
// Error message is stored in the buffer with pre-defined name err_msg; exception is the HRESULT code
#define CHECK_OK(code, msg)         \
    {                               \
        HRESULT res = code;         \
        if (res != S_OK)            \
        {                           \
            wcscpy_s(err_msg, msg); \
            throw res;              \
        }                           \
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

// Allocated a BSTR string using the specified text and checks for successful memory allocation
// Error message is stored in the buffer with pre-defined name err_msg; exception is 0 value
#define ALLOC_BSTR(name, str)                                               \
    BSTR name = SysAllocString(str);                                        \
    if (name == NULL)                                                       \
    {                                                                       \
        wcscpy_s(err_msg, L"Memory allocation for " ## L#name L" failed."); \
        throw (HRESULT)0;                                                   \
    }

// Releases the BSTR string and nullifies the pointer
#define FREE_BSTR(str)      \
    {                       \
        SysFreeString(str); \
        str = NULL;         \
    }


#endif // MAINDIALOG_H
