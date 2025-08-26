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
    property var selectedDevice: null
    property bool downloadComplete: false
    property string downloadedFilePath: ""
    property bool flashingStarted: false
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
                if (selectedDevice && !flashingStarted) {
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
            statusLabel.text = i18n("Starting download...");
            errorLabel.visible = false;
            successLabel.visible = false;
            retryButton.visible = false;
            fetchJob.fetch(isoUrl);
        }
    }

    function startFlashing() {
        if (downloadComplete && (verificationComplete || !isoHash) && selectedDevice && !flashingStarted) {
            flashingStarted = true;
            statusLabel.text = i18n("Starting flash...");
            flashController.startFlashing(downloadedFilePath, selectedDevice);
        }
    }

    Component.onCompleted: {
        // Auto-start download when page loads
        startDownload();
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Image {
            Layout.alignment: Qt.AlignHCenter
            width: Kirigami.Units.gridUnit * 4 // About 96px
            height: Kirigami.Units.gridUnit * 4
            source: "qrc:/qml/images/downloading.png"
            fillMode: Image.PreserveAspectFit
        }

        // ISO Information
        Kirigami.FormLayout {
            Layout.fillWidth: true

            Label {
                Kirigami.FormData.label: i18n("ISO:")
                text: isoName || i18n("Unknown ISO")
                elide: Label.ElideMiddle
                font.weight: Font.Medium
            }

            Label {
                Kirigami.FormData.label: i18n("USB Device:")
                text: selectedDevice ? selectedDevice.displayName : i18n("Unknown Device")
                elide: Label.ElideMiddle
                font.weight: Font.Medium
            }
        }

        // Progress Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Kirigami.Separator {
                Layout.fillWidth: true
                Layout.topMargin: Kirigami.Units.smallSpacing
            }

            Label {
                text: i18n("Progress")
                font.bold: true
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            }

            Label {
                id: statusLabel
                text: i18n("Ready to start...")
                Layout.fillWidth: true
                color: Kirigami.Theme.textColor
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
                font.weight: Font.Medium
            }

            Label {
                id: successLabel
                text: i18n("✅ Operation completed successfully!")
                Layout.fillWidth: true
                color: Kirigami.Theme.positiveTextColor
                visible: false
                font.weight: Font.Medium
            }

            Button {
                id: retryButton
                text: i18n("Retry")
                icon.name: "view-refresh"
                visible: false
                Layout.alignment: Qt.AlignLeft
                onClicked: {
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

        // Spacer to push everything up
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
                id: cancelButton
                text: i18n("Cancel")
                icon.name: "process-stop"
                onClicked: {
                    if (!downloadComplete && !flashingStarted) {
                        // Cancel download
                        fetchJob.cancel();
                    } else if (flashingStarted && !successLabel.visible && !errorLabel.visible) {
                        // Cancel flash
                        flashController.cancelFlashing();
                        statusLabel.text = i18n("Flash cancelled");
                    }
                    applicationWindow().pageStack.pop(null); // Go back to welcome page
                }
                visible: !successLabel.visible
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