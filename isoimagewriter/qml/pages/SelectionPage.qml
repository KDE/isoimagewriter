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

Kirigami.Page {
    id: selectionPage
    title: i18n("ISO Image Writer")

    property string selectedIsoPath: ""
    property string preselectedFile: ""
    property bool showVerifySection: false
    property bool isVerifying: false
    property string verificationResult: ""

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

    function verifyIsoIntegrity(filePath, expectedSha256) {
        isVerifying = true;
        verificationResult = i18n("Computing SHA256 checksum...");

        isoVerifier.filePath = filePath;
        isoVerifier.verifyWithSha256Sum(expectedSha256);
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

        // Verification Section
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: showVerifySection

            Label {
                text: i18n("Verify ISO integrity:")
                font.bold: true
            }

            TextField {
                id: sha256Field
                Layout.fillWidth: true
                placeholderText: i18n("Enter SHA256 checksum...")
                enabled: !isVerifying
            }

            RowLayout {
                Layout.fillWidth: true

                Button {
                    text: i18nc("@action:button", "Verify")
                    icon.name: "security-high"
                    enabled: sha256Field.text.length > 0 && selectedIsoPath !== "" && !isVerifying
                    onClicked: {
                    console.log("Length of SHA256 field:", sha256Field.text.length)
                        verifyIsoIntegrity(selectedIsoPath, sha256Field.text.trim());
                    }
                }

                Button {
                    text: i18nc("@action:button", "Cancel")
                    icon.name: "dialog-cancel"
                    enabled: !isVerifying
                    onClicked: {
                        showVerifySection = false;
                        sha256Field.text = "";
                        verificationResult = "";
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                BusyIndicator {
                    visible: isVerifying
                    running: isVerifying
                }
            }

            Label {
                Layout.fillWidth: true
                text: verificationResult
                color: verificationResult.includes("✓") ? Kirigami.Theme.positiveTextColor : verificationResult.includes("✗") ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
                wrapMode: Label.WordWrap
                visible: verificationResult !== ""
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
                enabled: usbDeviceModel && usbDeviceModel.hasDevices

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
                text: i18nc("@action:button", "Verify")
                icon.name: "security-medium"
                enabled: selectedIsoPath !== "" && !isVerifying
                onClicked: {
                    showVerifySection = true;
                }
            }

            Button {
                text: i18nc("@action:button", "Next")
                icon.name: "go-next"
                highlighted: true
                enabled: selectedIsoPath !== "" && deviceCombo.currentIndex >= 0
                onClicked: {
                    console.log("SelectionPage: Navigating to ProgressPage");
                    console.log("SelectionPage: Selected device index:", deviceCombo.currentIndex);

                    pageStack.push("qrc:/qml/pages/ProgressPage.qml", {
                        isoPath: selectedIsoPath,
                        selectedDeviceIndex: deviceCombo.currentIndex
                    });
                }
            }
        }
    }
}
