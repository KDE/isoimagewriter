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

#ifndef MAINDIALOG_H
#define MAINDIALOG_H

////////////////////////////////////////////////////////////////////////////////
// MainDialog is the main application window

#include <KAuth>
#include <KPixmapSequenceOverlayPainter>

#include <QDialog>

#include "common.h"
#include "externalprogressbar.h"

namespace Ui {
    class MainDialog;
}

enum VerificationResult { Fine, DinnaeKen, Invalid };
struct IsoResult {
    VerificationResult resultType;
    QString error;
};

class MainDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit MainDialog(QWidget *parent = 0);
    ~MainDialog();

private:
    Ui::MainDialog *ui;
    QPushButton *m_writeButton, *m_clearButton, *m_cancelButton;
    KAuth::ExecuteJob *m_job;
    IsoResult verifyISO();

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
    // Starts writing data to the device
    void writeToDeviceKAuth(bool zeroing);
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

    KPixmapSequenceOverlayPainter *m_busyWidget;

public slots:
    // Suggests to select image file using the Open File dialog
    void openImageFile();
    // Schedules reloading the list of USB flash disks to run when possible
    void scheduleEnumFlashDevices();
    // Starts writing the image
    void writeImageToDevice();
    // Clears the selected USB device
    void clearDevice();

    // Updates GUI to the "writing" mode (progress bar shown, controls disabled)
    // Also sets the progress bar limits
    void showWritingProgress(int maxValue);
    // Updates GUI to the "idle" mode (progress bar hidden, controls enabled)
    void hideWritingProgress();
    // Increments the progress bar counter by the specified number
    void updateProgressBar(int increment);
    // Displays the message about successful completion and returns to the "idle" mode
    void showSuccessMessage(QString msg);
    // Displays the specified error message and returns to the "idle" mode
    void showErrorMessage(QString msg);
    //cancel button clicked
    void cancelWriting();
    void progressStep(KJob* job, unsigned long step);
    void progressStep(const QVariantMap &);
    void statusChanged(KAuth::Action::AuthStatus status);
    void finished(KJob* job);
};


#endif // MAINDIALOG_H
