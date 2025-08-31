/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.isoimagewriter
import QtQuick.Window

FormCard.FormCardPage {
    id: selectionPage
    title: i18nc("@title:window", "ISO Image Writer")

    property string selectedIsoPath: ""
    property string preselectedFile: ""
    property bool isVerifying: false
    property string verificationResult: ""
    property var selectedDevice: null

    onSelectedIsoPathChanged: {
        console.log("SelectionPage: selectedIsoPath changed to:", selectedIsoPath);
    }

    Component.onCompleted: {
        console.log("SelectionPage: Component.onCompleted, preselectedFile:", preselectedFile);
        if (preselectedFile) {
            selectedIsoPath = preselectedFile;
            console.log("SelectionPage: Set selectedIsoPath to:", selectedIsoPath);
        }
    }

    onPreselectedFileChanged: {
        console.log("SelectionPage: preselectedFile changed to:", preselectedFile);
        if (preselectedFile) {
            selectedIsoPath = preselectedFile;
            console.log("SelectionPage: Updated selectedIsoPath to:", selectedIsoPath);
        }
    }

    IsoVerifier {
        id: isoVerifier

        onFinished: function (result, error) {
            isVerifying = false;

            if (result === IsoVerifier.Successful) {
                selectionPage.verificationResult = "✓ " + i18nc("@info:status", "Verification successful! ISO integrity confirmed.");
            } else {
                selectionPage.verificationResult = "✗ " + i18nc("@info:status", "Verification failed: %1", error);
            }
        }
    }

    function verifyIsoIntegrity(filePath, expectedSha256) {
        isVerifying = true;
        verificationResult = i18nc("@info:progress", "Computing SHA256 checksum…");

        isoVerifier.filePath = filePath;
        isoVerifier.verifyWithSha256Sum(expectedSha256);
    }

    function navigateToFlashPage() {
        if (!selectedDevice || !selectedDevice.physicalDevice) {
            console.error("SelectionPage: Invalid device selection");
            return;
        }

        console.log("SelectionPage: Navigating to flash page with device:", selectedDevice.physicalDevice);
        applicationWindow().pageStack.push("qrc:/qml/pages/FlashPage.qml", {
            "isoPath": selectedIsoPath,
            "selectedDevice": selectedDevice
        });
    }

    // SHA256 Input Dialog
    Kirigami.Dialog {
        id: sha256Dialog
        title: i18nc("@title:window", "Verify ISO Integrity")
        standardButtons: Kirigami.Dialog.NoButton

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            Controls.Label {
                Layout.fillWidth: true
                text: i18nc("@info", "Enter the expected SHA256 checksum to verify the integrity of your ISO file:")
                wrapMode: Text.WordWrap
            }

            Controls.TextField {
                id: sha256Input
                Layout.fillWidth: true
                placeholderText: i18nc("@info:placeholder", "SHA256 checksum…")
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

    // Confirmation Dialog
    Kirigami.Dialog {
        id: confirmationDialog
        title: i18nc("@title:window", "Warning: Data Will Be Erased")
        standardButtons: Kirigami.Dialog.NoButton

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            RowLayout {
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: "dialog-warning"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large
                    color: Kirigami.Theme.negativeTextColor
                }

                Controls.Label {
                    Layout.fillWidth: true
                    text: i18nc("@info", "All data on the selected USB device will be permanently erased!\n\nThis action cannot be undone. Please make sure you have backed up any important data before proceeding.")
                    wrapMode: Text.WordWrap
                    font.bold: true
                }
            }
        }

        customFooterActions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Abort")
                icon.name: "dialog-cancel"
                onTriggered: {
                    confirmationDialog.close();
                }
            },
            Kirigami.Action {
                text: i18nc("@action:button", "Continue")
                icon.name: "go-next"
                onTriggered: {
                    confirmationDialog.close();
                    navigateToFlashPage();
                }
            }
        ]
    }

    FileDialogBridge {
        id: fileDialog
    }

    // USB Device Selection Dialog
    Kirigami.Dialog {
        id: usbSelectionDialog
        title: i18nc("@title:window", "Select USB Device")
        standardButtons: Kirigami.Dialog.NoButton

        property var selectedDevice: null

        signal deviceSelected(var device)

        onDeviceSelected: function (device) {
            selectionPage.selectedDevice = device;
        }

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing
            implicitWidth: Kirigami.Units.gridUnit * 25

            FormCard.FormHeader {
                title: i18nc("@title:group", "Available USB Devices")
            }

            FormCard.FormCard {
                Repeater {
                    model: usbDeviceModel || null

                    FormCard.FormButtonDelegate {
                        required property string displayName
                        required property int index

                        text: displayName || ""
                        icon.name: "drive-removable-media"

                        onClicked: {
                            let selectedDevice = usbDeviceModel.getDevice(index);
                            usbSelectionDialog.deviceSelected(selectedDevice);
                            usbSelectionDialog.close();
                        }
                    }
                }
            }

            // Show message when no devices available
            Controls.Label {
                Layout.fillWidth: true
                text: i18nc("@info", "No USB devices detected. Please connect a USB drive and try again.")
                color: Kirigami.Theme.disabledTextColor
                horizontalAlignment: Text.AlignHCenter
                visible: !usbDeviceModel || !usbDeviceModel.hasDevices
                wrapMode: Text.WordWrap
            }
        }

        customFooterActions: [
            Kirigami.Action {
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                onTriggered: usbSelectionDialog.close()
            }
        ]
    }

    ColumnLayout {
        spacing: 0

        // ISO Selection Section
        FormCard.FormHeader {
            title: i18nc("@title:group", "ISO Image")
        }

        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                id: isoSelectionDelegate
                text: selectedIsoPath ? selectedIsoPath.split('/').pop() : i18nc("@action:button", "Select ISO file…")
                description: selectedIsoPath ? selectedIsoPath : i18nc("@info", "Choose an ISO image to write")
                icon.name: selectedIsoPath ? "application-x-cd-image" : "folder-open"
                onClicked: {
                    var fileUrl = fileDialog.selectImageFile();
                    if (fileUrl.toString() !== "") {
                        let filePath = fileUrl.toString();
                        if (filePath.startsWith("file://")) {
                            filePath = filePath.substring(7);
                        }
                        selectedIsoPath = filePath;
                    }
                }

                Component.onCompleted: {
                    console.log("FormButtonDelegate: selectedIsoPath is:", selectedIsoPath);
                }
            }
        }

        // USB Device Selection Section
        FormCard.FormHeader {
            title: i18nc("@title:group", "USB Device")
        }

        // Show USB selector when devices are available
        FormCard.FormCard {
            visible: usbDeviceModel && usbDeviceModel.hasDevices

            FormCard.FormButtonDelegate {
                id: usbSelectionDelegate
                text: selectedDevice ? selectedDevice.displayName : i18nc("@action:button", "Select USB device…")
                description: selectedDevice ? (selectedDevice.size ? i18nc("@info", "Size: %1", selectedDevice.size) : i18nc("@info", "USB device selected")) : i18nc("@info", "Choose a USB device to write to")
                icon.name: selectedDevice ? "drive-removable-media" : "drive-removable-media-usb"
                onClicked: usbSelectionDialog.open()
            }
        }

        // Warning message when no USB devices - replaces the FormCard
        FormCard.FormCard {
            visible: !usbDeviceModel || !usbDeviceModel.hasDevices

            RowLayout {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: "dialog-information"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.fillWidth: true
                        text: i18nc("@info", "No USB devices detected")
                        font.bold: true
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: i18nc("@info", "Please connect a USB drive to continue")
                        color: Kirigami.Theme.disabledTextColor
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }
    }

    footer: Controls.ToolBar {
        contentItem: RowLayout {
            Controls.Label {
                text: {
                    if (!selectedIsoPath) {
                        return "";
                    } else if (verificationResult.includes("✓")) {
                        return i18nc("@info:status", "Verified ISO Image");
                    } else if (verificationResult.includes("✗")) {
                        return i18nc("@info:status", "Verification Failed");
                    } else {
                        return i18nc("@info:status", " 🚫 Unverified ISO Image");
                    }
                }
                color: {
                    if (verificationResult.includes("✓")) {
                        return Kirigami.Theme.positiveTextColor;
                    } else if (verificationResult.includes("✗")) {
                        return Kirigami.Theme.negativeTextColor;
                    } else {
                        return Kirigami.Theme.disabledTextColor;
                    }
                }
                visible: selectedIsoPath !== ""
            }

            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18nc("@action:button", "Verify")
                icon.name: "security-medium"
                enabled: selectedIsoPath !== "" && !isVerifying
                onClicked: {
                    sha256Dialog.open();
                }
            }

            Controls.Button {
                text: i18nc("@action:button", "Next")
                icon.name: "go-next"
                highlighted: true
                enabled: selectedIsoPath !== "" && selectedDevice && !isVerifying
                onClicked: {
                    confirmationDialog.open();
                }
            }
        }
    }
}
