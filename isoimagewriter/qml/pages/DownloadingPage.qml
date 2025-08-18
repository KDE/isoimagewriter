/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts
import org.kde.isoimagewriter 1.0

Kirigami.Page {
    id: downloadingPage
    title: i18n("Download & Flash ISO")

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""
    property bool downloadComplete: false
    property string downloadedFilePath: ""
    property bool flashingStarted: false
    property bool showProgress: false
    property bool verificationComplete: false

    FetchIsoJob {
        id: fetchJob

        onDownloadProgressChanged: function (percentage) {
            downloadProgressBar.value = percentage;
            statusLabel.text = i18n("Downloading: %1%", percentage);
        }

        onFinished: function (filePath) {
            downloadComplete = true;
            downloadedFilePath = filePath;
            statusLabel.text = i18n("Download completed!");

            // Start SHA256 verification if hash is available
            if (isoHash && isoHashAlgo === "sha256") {
                statusLabel.text = i18n("Verifying SHA256 checksum...");
                isoVerifier.filePath = filePath;
                isoVerifier.verifyWithInputText(true, isoHash);
            }
        }

        onFailed: {
            statusLabel.text = i18n("Download failed!");
            errorLabel.text = i18n("Failed to download the ISO file. Please try again.");
            errorLabel.visible = true;
            retryButton.visible = true;
        }
    }

    IsoVerifier {
        id: isoVerifier

        onFinished: function (result, error) {
            if (result === IsoVerifier.Successful) {
                verificationComplete = true;
                statusLabel.text = i18n("SHA256 verification successful!");

                // Auto-start flashing if USB device is selected
                if (usbDeviceCombo.currentIndex >= 0 && !flashingStarted) {
                    startFlashing();
                }
            } else {
                statusLabel.text = i18n("SHA256 verification failed!");
                errorLabel.text = error || i18n("The downloaded file's checksum does not match the expected value.");
                errorLabel.visible = true;
                retryButton.visible = true;
            }
        }
    }

    FlashController {
        id: flashController

        onProgressChanged: {
            flashProgressBar.value = progress;
            statusLabel.text = i18n("Flashing: %1%", Math.round(progress));
        }

        onFlashCompleted: {
            statusLabel.text = i18n("Flash completed successfully!");
            successLabel.visible = true;
        }

        onFlashFailed: function (error) {
            statusLabel.text = i18n("Flash failed!");
            errorLabel.text = error;
            errorLabel.visible = true;
            retryButton.visible = true;
        }
    }

    function startDownload() {
        if (isoUrl) {
            showProgress = true;
            statusLabel.text = i18n("Starting download...");
            errorLabel.visible = false;
            successLabel.visible = false;
            retryButton.visible = false;
            fetchJob.fetch(isoUrl);
        }
    }

    function startFlashing() {
        if (downloadComplete && (verificationComplete || !isoHash) && usbDeviceCombo.currentIndex >= 0 && !flashingStarted) {
            flashingStarted = true;
            let device = usbDeviceModel.getDevice(usbDeviceCombo.currentIndex);
            if (device) {
                statusLabel.text = i18n("Starting flash...");
                flashController.startFlashing(downloadedFilePath, device);
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: i18n("ISO to download:")
                font.bold: true
            }

            Label {
                text: isoName || i18n("Unknown ISO")
                Layout.fillWidth: true
                elide: Label.ElideMiddle
            }
        }

        // Row 2: USB Drive Selection
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: i18n("Select USB drive:")
                font.bold: true
            }

            ComboBox {
                id: usbDeviceCombo
                Layout.fillWidth: true
                model: usbDeviceModel || null
                textRole: "displayName"
                enabled: usbDeviceModel && usbDeviceModel.hasDevices && !flashController.isWriting

                // Auto-select first device if available
                currentIndex: (usbDeviceModel && usbDeviceModel.hasDevices && count > 0) ? 0 : -1

                delegate: ItemDelegate {
                    width: usbDeviceCombo.width
                    text: model.displayName
                    highlighted: usbDeviceCombo.highlightedIndex === index
                }

                // Show placeholder text when no devices
                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: usbDeviceCombo.leftPadding
                    anchors.verticalCenter: parent.verticalCenter
                    text: i18n("Please plug in a USB drive")
                    color: Kirigami.Theme.disabledTextColor
                    visible: !usbDeviceModel || !usbDeviceModel.hasDevices || usbDeviceCombo.count === 0
                }
            }

            Label {
                Layout.fillWidth: true
                text: i18n("All data on the selected device will be permanently erased!")
                color: Kirigami.Theme.negativeTextColor
                wrapMode: Label.WordWrap
                visible: usbDeviceCombo.currentIndex >= 0
            }
        }

        // Row 3: Progress Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: showProgress

            Label {
                id: statusLabel
                text: i18n("Ready to start...")
                Layout.fillWidth: true
            }

            ProgressBar {
                id: downloadProgressBar
                Layout.fillWidth: true
                from: 0
                to: 100
                value: 0
                visible: !downloadComplete
            }

            ProgressBar {
                id: flashProgressBar
                Layout.fillWidth: true
                from: 0
                to: 100
                value: 0
                visible: downloadComplete && flashingStarted
            }

            Label {
                id: errorLabel
                Layout.fillWidth: true
                color: Kirigami.Theme.negativeTextColor
                wrapMode: Label.WordWrap
                visible: false
            }

            Label {
                id: successLabel
                text: i18n("Operation completed successfully!")
                Layout.fillWidth: true
                color: Kirigami.Theme.positiveTextColor
                visible: false
            }

            Button {
                id: retryButton
                text: i18n("Retry")
                icon.name: "view-refresh"
                visible: false
                onClicked: {
                    showProgress = false;
                    downloadComplete = false;
                    verificationComplete = false;
                    flashingStarted = false;
                    downloadProgressBar.value = 0;
                    flashProgressBar.value = 0;
                    errorLabel.visible = false;
                    successLabel.visible = false;
                    retryButton.visible = false;
                    startDownload();
                }
            }
        }

        // Spacer
        Item {
            Layout.fillHeight: true
        }
    }

    footer: ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Button {
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                onClicked: {
                    applicationWindow().pageStack.pop();
                }
                visible: !showProgress || errorLabel.visible
            }

            Button {
                id: cancelDownloadButton
                text: i18n("Cancel Download")
                icon.name: "process-stop"
                onClicked: {
                    // Cancel the actual download
                    fetchJob.cancel();
                    applicationWindow().pageStack.pop(null); // Go back to welcome page

                    // Reset the download state
                    // showProgress = false
                    // downloadComplete = false
                    // flashingStarted = false
                    // downloadProgressBar.value = 0
                    // flashProgressBar.value = 0
                    // statusLabel.text = i18n("Download cancelled")
                    // errorLabel.visible = false
                    // successLabel.visible = false
                    // retryButton.visible = false
                }
                visible: showProgress && !downloadComplete && !flashingStarted && !errorLabel.visible && !successLabel.visible
            }

            Button {
                id: cancelFlashButton
                text: i18n("Cancel Flash")
                icon.name: "process-stop"
                onClicked: {
                    flashController.cancelFlashing();
                    statusLabel.text = i18n("Flash cancelled");
                    showProgress = false;
                    errorLabel.visible = false;
                    successLabel.visible = false;
                    retryButton.visible = false;
                }
                visible: showProgress && flashingStarted && !errorLabel.visible && !successLabel.visible
            }

            Button {
                id: startButton
                text: i18n("Start Download & Flash")
                icon.name: "media-flash"
                highlighted: true
                enabled: isoUrl && usbDeviceCombo.currentIndex >= 0 && !showProgress
                onClicked: {
                    startDownload();
                }
                visible: !showProgress || errorLabel.visible
            }

            Button {
                text: i18n("Done")
                icon.name: "dialog-ok"
                highlighted: true
                onClicked: {
                    applicationWindow().pageStack.pop(null); // Go back to welcome page
                }
                visible: successLabel.visible
            }
        }
    }
}
