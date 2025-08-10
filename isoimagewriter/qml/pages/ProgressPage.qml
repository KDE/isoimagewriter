/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

import org.kde.isoimagewriter 1.0

Kirigami.ScrollablePage {
    id: progressPage
    title: i18n("Flash USB Drive")

    property string isoPath: ""
    property int selectedDeviceIndex: -1

    Component.onCompleted: {
        console.log("ProgressPage: Ready to flash");
        console.log("ProgressPage: ISO path:", isoPath);
        console.log("ProgressPage: Device index:", selectedDeviceIndex);
    }

    // Real FlashController instance from C++
    FlashController {
        id: flashController

        onFlashCompleted: {
            console.log("Flash completed successfully!");
            startButton.visible = false;
            homeButton.visible = true;
        }

        onFlashFailed: function (error) {
            console.error("Flash failed:", error);
            startButton.visible = true;
            startButton.enabled = true;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Row 1: Flashing info
        Label {
            Layout.fillWidth: true
            text: i18n("Flashing: %1", isoPath.split('/').pop() || i18n("No file selected"))
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            font.weight: Font.Bold
            wrapMode: Label.WordWrap
        }

        // Row 2: Target device info
        Label {
            Layout.fillWidth: true
            text: {
                if (selectedDeviceIndex >= 0 && usbDeviceModel && selectedDeviceIndex < usbDeviceModel.count) {
                    // Get the device directly and access its displayName (same as ComboBox approach)
                    let device = usbDeviceModel.getDevice(selectedDeviceIndex);
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

        // Spacer to push everything to top
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
                onClicked: {
                    if (flashController.isWriting) {
                        flashController.cancelFlashing();
                    }
                    pageStack.pop();
                }
                visible: flashController.isWriting
            }

            // Big Start Flashing Button
            Button {
                id: startButton
                text: i18n("START FLASHING")
                icon.name: "media-flash"
                highlighted: true
                enabled: isoPath && selectedDeviceIndex >= 0 && !flashController.isWriting
                visible: !flashController.isWriting

                onClicked: {
                    console.log("ProgressPage: Starting flash operation");
                    
                    // Use the exact same approach as FlashPage.qml
                    if (selectedDeviceIndex < 0 || !usbDeviceModel) {
                        console.error("ProgressPage: Invalid device selection");
                        return;
                    }

                    let device = usbDeviceModel.getDevice(selectedDeviceIndex);
                    if (device && device.physicalDevice) {
                        console.log("ProgressPage: Starting flash with device:", device.physicalDevice);
                        flashController.startFlashing(isoPath, device);
                    } else {
                        console.error("ProgressPage: No valid device selected");
                    }
                }

                // Make the button look prominent
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                font.weight: Font.Bold
            }

            Button {
                id: homeButton
                text: i18nc("@action:button", "Home")
                icon.name: "go-home"
                onClicked: {
                    // Go back to the first page (WelcomePage)
                    pageStack.pop(null);
                }
                visible: false // Only show after successful completion
            }
        }
    }
}
