/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs
import org.kde.kirigami as Kirigami
import org.kde.isoimagewriter
import QtQuick.Window

Kirigami.Page {
    id: selectionPage
    title: i18n("ISO Image Writer")

    property string selectedIsoPath: ""
    property string preselectedFile: ""
    property bool isVerifying: false
    property string verificationResult: ""
    property bool isFlashing: false

    Component.onCompleted: {
        if (preselectedFile) {
            selectedIsoPath = preselectedFile;
        }
    }

    IsoVerifier {
        id: isoVerifier

        onFinished: function (result, error) {
            isVerifying = false;

            if (result === IsoVerifier.Successful) {
                selectionPage.verificationResult = "✓ " + i18n("Verification successful! ISO integrity confirmed.");
            } else {
                selectionPage.verificationResult = "✗ " + i18n("Verification failed: %1", error);
            }
        }
    }

    FlashController {
        id: flashController

        onFlashCompleted: {
            console.log("Flash completed successfully!");
            isFlashing = false;
        }

        onFlashFailed: function (error) {
            console.error("Flash failed:", error);
            isFlashing = false;
        }
    }

    function verifyIsoIntegrity(filePath, expectedSha256) {
        isVerifying = true;
        verificationResult = i18n("Computing SHA256 checksum...");

        isoVerifier.filePath = filePath;
        isoVerifier.verifyWithSha256Sum(expectedSha256);
    }

    function startFlashing() {
        if (deviceCombo.currentIndex < 0 || !usbDeviceModel) {
            console.error("SelectionPage: Invalid device selection");
            return;
        }

        let device = usbDeviceModel.getDevice(deviceCombo.currentIndex);
        if (device && device.physicalDevice) {
            console.log("SelectionPage: Starting flash with device:", device.physicalDevice);
            isFlashing = true;
            flashController.startFlashing(selectedIsoPath, device);
        } else {
            console.error("SelectionPage: No valid device selected");
        }
    }

    // SHA256 Input Dialog
    Kirigami.Dialog {
        id: sha256Dialog
        title: i18n("Verify ISO Integrity")
        standardButtons: Kirigami.Dialog.NoButton

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            Label {
                Layout.fillWidth: true
                text: i18n("Enter the expected SHA256 checksum to verify the integrity of your ISO file:")
                wrapMode: Label.WordWrap
            }

            TextField {
                id: sha256Input
                Layout.fillWidth: true
                placeholderText: i18n("SHA256 checksum...")
                selectByMouse: true
            }
        }

        customFooterActions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                onTriggered: {
                    sha256Dialog.close();
                    sha256Input.text = "";
                }
            },
            Kirigami.Action {
                text: i18nc("@action:button", "Verify")
                icon.name: "security-medium"
                enabled: sha256Input.text.trim().length > 0
                onTriggered: {
                    verifyIsoIntegrity(selectedIsoPath, sha256Input.text.trim());
                    sha256Dialog.close();
                    sha256Input.text = "";
                }
            }
        ]
    }

    FileDialog {
        id: fileDialog
        title: i18n("Select ISO image")
        nameFilters: ["ISO files (*.iso)", "All files (*)"]
        modality: Qt.WindowModal
        options: FileDialog.DontUseNativeDialog
        onAccepted: {
            if (fileDialog.selectedFile) {
                let filePath = fileDialog.selectedFile.toString();
                if (filePath.startsWith("file://")) {
                    filePath = filePath.substring(7);
                }
                selectedIsoPath = filePath;
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Row 1: ISO Selection
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Label {
                text: i18n("Select ISO image:")
                font.bold: true
            }

            TextField {
                id: isoField
                Layout.fillWidth: true
                placeholderText: i18n("Click to select ISO file…")
                text: selectedIsoPath
                readOnly: true
                rightPadding: folderButton.width + Kirigami.Units.smallSpacing

                MouseArea {
                    anchors.fill: parent
                    onClicked: fileDialog.open()
                    cursorShape: Qt.PointingHandCursor
                }

                Button {
                    id: folderButton
                    anchors.right: parent.right
                    anchors.rightMargin: Kirigami.Units.smallSpacing
                    anchors.verticalCenter: parent.verticalCenter
                    icon.name: "folder-open"
                    flat: true
                    onClicked: fileDialog.open()

                    // Make button smaller to fit nicely inside text field
                    implicitWidth: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing
                    implicitHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing
                }
            }
        }

        // Verification Result Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: verificationResult !== "" || isVerifying

            RowLayout {
                Layout.fillWidth: true

                Label {
                    Layout.fillWidth: true
                    text: isVerifying ? i18n("Computing SHA256 checksum...") : verificationResult
                    color: verificationResult.includes("✓") ? Kirigami.Theme.positiveTextColor : verificationResult.includes("✗") ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                    wrapMode: Label.WordWrap
                }

                BusyIndicator {
                    visible: isVerifying
                    running: isVerifying
                }
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
                id: deviceCombo
                Layout.fillWidth: true
                model: usbDeviceModel || null
                textRole: "displayName"
                enabled: usbDeviceModel && usbDeviceModel.hasDevices && !isFlashing

                // Auto-select first device if available
                currentIndex: (usbDeviceModel && usbDeviceModel.hasDevices && count > 0) ? 0 : -1

                delegate: ItemDelegate {
                    width: deviceCombo.width
                    text: model.displayName
                    highlighted: deviceCombo.highlightedIndex === index
                }

                // Show placeholder text when no devices
                Label {
                    anchors.left: parent.left
                    anchors.leftMargin: deviceCombo.leftPadding
                    anchors.verticalCenter: parent.verticalCenter
                    text: i18n("Please plug in a USB drive")
                    color: Kirigami.Theme.disabledTextColor
                    visible: !usbDeviceModel || !usbDeviceModel.hasDevices || deviceCombo.count === 0
                }
            }

            Label {
                Layout.fillWidth: true
                text: i18n("All data on the selected device will be permanently erased!")
                color: Kirigami.Theme.negativeTextColor
                wrapMode: Label.WordWrap
                visible: deviceCombo.currentIndex >= 0
            }
        }

        // Flash Progress Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: isFlashing

            Label {
                Layout.fillWidth: true
                text: i18n("Flashing: %1", selectedIsoPath.split('/').pop() || i18n("No file selected"))
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                font.weight: Font.Bold
                wrapMode: Label.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: {
                    if (deviceCombo.currentIndex >= 0 && usbDeviceModel && deviceCombo.currentIndex < usbDeviceModel.count) {
                        let device = usbDeviceModel.getDevice(deviceCombo.currentIndex);
                        if (device && device.displayName) {
                            return i18n("Into: %1", device.displayName);
                        } else {
                            return i18n("Into: %1", i18n("Unknown device"));
                        }
                    } else {
                        return i18n("Into: %1", i18n("No device selected"));
                    }
                }
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Label.WordWrap
            }

            Label {
                Layout.fillWidth: true
                text: flashController.statusMessage || i18n("Preparing...")
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                wrapMode: Label.WordWrap
            }

            ProgressBar {
                Layout.fillWidth: true
                value: flashController.progress
                from: 0.0
                to: 1.0
            }

            Label {
                Layout.fillWidth: true
                text: i18n("Progress: %1%", Math.round(flashController.progress * 100))
                color: Kirigami.Theme.disabledTextColor
                horizontalAlignment: Text.AlignHCenter
            }

            // Error message section
            Label {
                Layout.fillWidth: true
                text: flashController.errorMessage
                color: Kirigami.Theme.negativeTextColor
                font.weight: Font.Bold
                wrapMode: Label.WordWrap
                visible: flashController.errorMessage.length > 0
            }

            // Success message section
            Label {
                Layout.fillWidth: true
                text: i18n("Flash completed successfully!")
                color: Kirigami.Theme.positiveTextColor
                font.weight: Font.Bold
                wrapMode: Label.WordWrap
                visible: !flashController.isWriting && flashController.progress >= 1.0 && flashController.errorMessage.length === 0
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
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                visible: isFlashing
                onClicked: {
                    if (flashController.isWriting) {
                        flashController.cancelFlashing();
                    }
                    isFlashing = false;
                }
            }

            Button {
                text: i18nc("@action:button", "Verify")
                icon.name: "security-medium"
                enabled: selectedIsoPath !== "" && !isVerifying && !isFlashing
                visible: !isFlashing
                onClicked: {
                    sha256Dialog.open();
                }
            }

            Button {
                text: i18nc("@action:button", "Home")
                icon.name: "go-home"
                visible: !flashController.isWriting && flashController.progress >= 1.0 && flashController.errorMessage.length === 0
                onClicked: {
                    // Go back to the first page (WelcomePage)
                    pageStack.pop(null);
                    // Reset state
                    isFlashing = false;
                    verificationResult = "";
                }
            }

            Button {
                text: i18nc("@action:button", "START FLASHING")
                icon.name: "media-flash"
                highlighted: true
                enabled: selectedIsoPath !== "" && deviceCombo.currentIndex >= 0 && !isFlashing && !isVerifying
                visible: !isFlashing || (flashController.errorMessage.length > 0)
                onClicked: {
                    startFlashing();
                }
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
                font.weight: Font.Bold
            }
        }
    }
}
