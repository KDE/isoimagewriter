#ifndef MAINDIALOG_H
#define MAINDIALOG_H

////////////////////////////////////////////////////////////////////////////////
// MainDialog is the main application window


#include <QDialog>

#include "common.h"
#include "externalprogressbar.h"

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


#endif // MAINDIALOG_H
