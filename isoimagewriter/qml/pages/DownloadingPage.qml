/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.isoimagewriter 1.0
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: downloadingPage

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""
    property var selectedDevice: null
    property bool downloadComplete: false
    property string downloadedFilePath: ""
    property bool flashingStarted: false
    property string currentStatus: ""

    function startDownload() {
        if (isoUrl) {
            currentStatus = i18n("Starting download…");
            errorLabel.visible = false;
            successLabel.visible = false;
            fetchJob.fetch(isoUrl);
        }
    }

    function startFlashing() {
        if (downloadComplete && selectedDevice && !flashingStarted) {
            flashingStarted = true;
            currentStatus = i18n("Starting flash…");
            flashController.startFlashing(downloadedFilePath, selectedDevice);
        }
    }

    title: successLabel.visible ? i18n("USB Drive Ready") : errorLabel.visible ? i18n("Operation Failed") : downloadComplete && flashingStarted ? i18n("Creating USB Drive") : i18n("Downloading ISO")
    Component.onCompleted: {
        // Auto-start the download process
        currentStatus = i18n("Preparing to download…");
        Qt.callLater(startDownload);
    }

    FetchIsoJob {
        id: fetchJob

        onDownloadProgressChanged: function(percentage) {
            downloadProgressBar.value = percentage;
            currentStatus = i18n("Downloading: %1%", percentage);
        }
        onFinished: function(filePath) {
            downloadComplete = true;
            downloadedFilePath = filePath;
            currentStatus = i18n("Download completed! Starting flash…");
            // Skip SHA verification - directly start flashing
            if (selectedDevice && !flashingStarted)
                startFlashing();

        }
        onFailed: {
            currentStatus = i18n("Download failed!");
            errorLabel.text = i18n("Failed to download the ISO file. Please try again.");
            errorLabel.visible = true;
        }
    }

    FlashController {
        id: flashController

        onProgressChanged: {
            flashProgressBar.value = flashController.progress * 100;
            currentStatus = i18n("Flashing: %1%", Math.round(flashController.progress * 100));
        }
        onFlashCompleted: {
            currentStatus = i18n("Flash completed successfully!");
            successLabel.visible = true;
        }
        onFlashFailed: function(error) {
            currentStatus = i18n("Flash failed!");
            errorLabel.text = error;
            errorLabel.visible = true;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 40)
        spacing: Kirigami.Units.largeSpacing * 2

        // Success/Error/Status Icon
        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            source: successLabel.visible ? "checkmark" : errorLabel.visible ? "error" : downloadComplete && flashingStarted ? "media-optical" : "cloud-download"
            color: successLabel.visible ? Kirigami.Theme.positiveTextColor : errorLabel.visible ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
        }

        // Subtitle message
        Controls.Label {
            Layout.fillWidth: true
            text: successLabel.visible ? i18n("Your bootable USB drive has been created successfully!\nYou can now safely remove it and use it to boot your computer.") : errorLabel.visible ? (errorLabel.text || i18n("Please try again or check your connection.")) : currentStatus || (downloadComplete && flashingStarted ? i18n("Writing data to your USB drive. This may take some time.") : i18n("Downloading the ISO file. This may take some time depending on your internet connection."))
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
            color: successLabel.visible ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Progress bar with percentage
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: !successLabel.visible && !errorLabel.visible

            Controls.ProgressBar {
                id: downloadProgressBar

                Layout.fillWidth: true
                height: Kirigami.Units.gridUnit * 2
                from: 0
                to: 100
                value: 0
                visible: !downloadComplete

                // Progress percentage text overlay
                Controls.Label {
                    anchors.centerIn: parent
                    text: Math.round(downloadProgressBar.value) + "%"
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }

            }

            Controls.ProgressBar {
                id: flashProgressBar

                Layout.fillWidth: true
                height: Kirigami.Units.gridUnit * 2
                from: 0
                to: 100
                value: 0
                visible: downloadComplete && flashingStarted

                // Progress percentage text overlay
                Controls.Label {
                    anchors.centerIn: parent
                    text: Math.round(flashProgressBar.value) + "%"
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }

            }

            Controls.Label {
                Layout.fillWidth: true
                text: downloadComplete && flashingStarted ? i18n("Writing data to USB drive…") : i18n("Downloading ISO file…")
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
            }

        }

        // Error/Success labels (hidden, used for state checking)
        Controls.Label {
            id: errorLabel

            Layout.fillWidth: true
            color: Kirigami.Theme.negativeTextColor
            wrapMode: Text.WordWrap
            visible: false
            font.weight: Font.Medium
        }

        Controls.Label {
            id: successLabel

            text: i18n("✅ Operation completed successfully!")
            Layout.fillWidth: true
            color: Kirigami.Theme.positiveTextColor
            visible: false
            font.weight: Font.Medium
        }

        // Error actions (only show Retry button in main content when failed)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.mediumSpacing
            visible: errorLabel.visible

            Controls.Button {
                text: i18n("Retry")
                icon.name: "view-refresh"
                highlighted: true
                onClicked: {
                    downloadComplete = false;
                    flashingStarted = false;
                    downloadProgressBar.value = 0;
                    flashProgressBar.value = 0;
                    errorLabel.visible = false;
                    successLabel.visible = false;
                    startDownload();
                }
            }

        }

    }

    footer: Controls.ToolBar {

        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                onClicked: {
                    if (!downloadComplete && !flashingStarted) {
                        // Cancel download
                        fetchJob.cancel();
                    } else if (flashingStarted && !successLabel.visible && !errorLabel.visible) {
                        // Cancel flash
                        flashController.cancelFlashing();
                        currentStatus = i18n("Flash cancelled");
                    }
                    applicationWindow().pageStack.pop();
                }
                visible: !successLabel.visible
            }

            Controls.Button {
                text: i18n("Done")
                icon.name: "dialog-ok"
                highlighted: true
                onClicked: {
                    applicationWindow().pageStack.pop(null); // Go back to welcome page
                }
                visible: successLabel.visible
            }

            Controls.Button {
                text: i18n("Retry")
                icon.name: "view-refresh"
                highlighted: true
                onClicked: {
                    downloadComplete = false;
                    flashingStarted = false;
                    downloadProgressBar.value = 0;
                    flashProgressBar.value = 0;
                    errorLabel.visible = false;
                    successLabel.visible = false;
                    startDownload();
                }
                visible: errorLabel.visible
            }

        }

    }

}
