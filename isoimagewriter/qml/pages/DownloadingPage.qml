/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.isoimagewriter 1.0
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: downloadingPage
    title: downloadCompleted ? i18nc("@title:window", "USB Drive Ready") : downloadFailed ? i18nc("@title:window", "Download Failed") : flashingStarted ? i18nc("@title:window", "Creating USB Drive") : i18nc("@title:window", "Downloading ISO")
    
    // Disable header navigation during download/flashing
    globalToolBarStyle: (downloadingStarted && !downloadCompleted && !downloadFailed) ? 
                       Kirigami.ApplicationHeaderStyle.None : 
                       Kirigami.ApplicationHeaderStyle.Auto

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""
    property var selectedDevice: null
    property bool downloadingStarted: false
    property bool downloadCompleted: false
    property bool downloadFailed: false
    property string downloadedFilePath: ""
    property bool flashingStarted: false
    property bool flashCompleted: false
    property bool flashFailed: false
    property string errorMessage: ""
    property real downloadProgress: 0.0
    property real flashProgress: 0.0



    function startDownload() {
        if (isoUrl && !downloadingStarted) {
            downloadingStarted = true;
            downloadProgress = 0.0;
            console.log("DownloadingPage: Starting download");
            fetchJob.fetch(isoUrl);
        }
    }

    function startFlashing() {
        if (downloadCompleted && selectedDevice && !flashingStarted) {
            flashingStarted = true;
            console.log("DownloadingPage: Starting flash with device:", selectedDevice.physicalDevice);
            flashController.startFlashing(downloadedFilePath, selectedDevice);
        }
    }
    Component.onCompleted: {
        console.log("DownloadingPage: Ready to download ISO");
        // Auto-start the download process
        Qt.callLater(startDownload);
    }

    FetchIsoJob {
        id: fetchJob

        onDownloadProgressChanged: function(percentage) {
            console.log("DownloadingPage: Download progress", percentage + "%");
            downloadingPage.downloadProgress = percentage / 100.0;
        }

        onFinished: function(filePath) {
            console.log("DownloadingPage: Download completed");
            downloadingPage.downloadCompleted = true;
            downloadingPage.downloadFailed = false;
            downloadingPage.downloadedFilePath = filePath;
            // Auto-start flashing when download completes
            if (selectedDevice && !flashingStarted) {
                Qt.callLater(startFlashing);
            }
        }

        onFailed: function(error) {
            console.log("DownloadingPage: Download failed with error:", error);
            downloadingPage.downloadCompleted = false;
            downloadingPage.downloadFailed = true;
            downloadingPage.errorMessage = error;
        }
    }

    FlashController {
        id: flashController

        onProgressChanged: {
            console.log("DownloadingPage: Flash progress", Math.round(progress * 100) + "%");
            downloadingPage.flashProgress = progress;
        }

        onFlashCompleted: {
            console.log("DownloadingPage: Flash completed");
            downloadingPage.flashCompleted = true;
            downloadingPage.flashFailed = false;
        }

        onFlashFailed: function(error) {
            console.log("DownloadingPage: Flash failed with error:", error);
            downloadingPage.flashCompleted = false;
            downloadingPage.flashFailed = true;
            downloadingPage.errorMessage = error;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 40)
        spacing: Kirigami.Units.largeSpacing * 2

        // Success/Error Icon
        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            source: flashCompleted ? "checkmark" : (downloadFailed || flashFailed) ? "error" : flashingStarted ? "media-optical" : "cloud-download"
            color: flashCompleted ? Kirigami.Theme.positiveTextColor : (downloadFailed || flashFailed) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            visible: flashCompleted || downloadFailed || flashFailed || !downloadingStarted
        }

        // Main status message
        Controls.Label {
            Layout.fillWidth: true
            text: flashCompleted ? i18nc("@info:status", "✅ USB Drive Ready!") : (downloadFailed || flashFailed) ? i18nc("@info:status", "❌ Operation Failed") : flashingStarted ? i18nc("@info:progress", "Creating your bootable USB drive") : i18nc("@info:progress", "Downloading ISO file")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
            font.bold: flashCompleted || downloadFailed || flashFailed
            color: flashCompleted ? Kirigami.Theme.positiveTextColor : (downloadFailed || flashFailed) ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Subtitle message
        Controls.Label {
            Layout.fillWidth: true
            text: flashCompleted ? i18nc("@info", "Your bootable USB drive has been created successfully!\nYou can now safely remove it and use it to boot your computer.") : (downloadFailed || flashFailed) ? (errorMessage || i18nc("@info", "Please try again or check your connection.")) : flashingStarted ? i18nc("@info", "This may take some time depending on the size of the ISO image file and the transfer speed.") : i18nc("@info", "This may take some time depending on your internet connection.")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
            color: flashCompleted ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Progress bar with percentage
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: (downloadingStarted && !downloadCompleted && !downloadFailed) || (flashingStarted && !flashCompleted && !flashFailed)
            


            // Progress bar container with overlay text
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                Controls.ProgressBar {
                    id: progressBar
                    anchors.fill: parent
                    from: 0
                    to: 1
                    value: flashingStarted ? flashProgress : downloadProgress
                }

                // Progress percentage text overlay
                Controls.Label {
                    anchors.centerIn: parent
                    text: Math.round(progressBar.value * 100) + "%"
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }
            }

            Controls.Label {
                Layout.fillWidth: true
                text: flashingStarted ? i18nc("@info:progress", "Writing data to USB drive…") : i18nc("@info:progress", "Downloading ISO file…")
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
            }
        }

        // Error actions (only show Retry button in main content when failed)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.mediumSpacing
            visible: downloadFailed || flashFailed

            Controls.Button {
                text: i18nc("@action:button", "Retry")
                icon.name: "view-refresh"
                highlighted: true
                onClicked: {
                    downloadingStarted = false;
                    downloadCompleted = false;
                    downloadFailed = false;
                    flashingStarted = false;
                    flashCompleted = false;
                    flashFailed = false;
                    errorMessage = "";
                    downloadProgress = 0.0;
                    flashProgress = 0.0;
                    Qt.callLater(startDownload);
                }
            }
        }

    }

    // Footer with Cancel/Done button
    footer: Controls.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                visible: !flashCompleted && !downloadFailed && !flashFailed
                onClicked: {
                    if (downloadingStarted && !downloadCompleted && !downloadFailed) {
                        fetchJob.cancel();
                    } else if (flashingStarted && !flashCompleted && !flashFailed) {
                        flashController.cancelFlashing();
                    }
                    applicationWindow().pageStack.pop();
                }
            }

            Controls.Button {
                text: i18nc("@action:button", "Done")
                icon.name: "dialog-ok"
                highlighted: true
                visible: flashCompleted
                onClicked: {
                    Qt.quit();
                }
            }
        }
    }
}
