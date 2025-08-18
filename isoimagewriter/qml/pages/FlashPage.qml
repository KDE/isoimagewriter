/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

//TODO: if usb is removed while on this page then it should go back to the home page 
//TODO: FIX SEG fault error, go back from flashpage to FilePage


import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

import org.kde.isoimagewriter 1.0

Kirigami.ScrollablePage {
    id: flashPage
    title: i18n("Write to USB Drive")

    property string isoPath: "/path/to/your/selected/image.iso"
    property string preselectedDevicePath: ""

    Component.onCompleted: {
        console.log("FlashPage: Ready for device selection");

        // If a device path was passed, try to find and select it
        if (preselectedDevicePath && usbDeviceModel) {
            console.log("FlashPage: Looking for preselected device:", preselectedDevicePath);
            selectDeviceByPath(preselectedDevicePath);
        }
    }

    function selectDeviceByPath(devicePath) {
        if (!usbDeviceModel || !devicePath) {
            return;
        }

        // Search through all devices to find the one with matching physical path
        for (let i = 0; i < usbDeviceModel.count; i++) {
            let device = usbDeviceModel.getDevice(i);
            if (device && device.physicalDevice === devicePath) {
                deviceCombo.currentIndex = i;
                console.log("FlashPage: Found and selected device at index", i, ":", device.displayName);
                return;
            }
        }

        console.log("FlashPage: Could not find device with path:", devicePath);
    }

    // Real FlashController instance from C++
    FlashController {
        id: flashController

        onFlashCompleted: {
            successMessage.visible = true;
        }

        onFlashFailed: function (error) {
            console.error("Flash failed:", error);
        }

        Component.onDestruction: {
            console.log("FlashController is being destroyed")
            // Add any cleanup if needed
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Information")
        }

        Label {
            Kirigami.FormData.label: i18n("Source ISO:")
            text: isoPath.split('/').pop() || i18n("No file selected")
            elide: Label.ElideMiddle
        }

        ComboBox {
            id: deviceCombo
            Kirigami.FormData.label: i18n("Target Device:")
            Layout.fillWidth: true

            model: usbDeviceModel
            textRole: "displayName"
            enabled: usbDeviceModel && usbDeviceModel.hasDevices && !flashController.isWriting

            // Auto-select first device if available and no preselection
            currentIndex: (usbDeviceModel && usbDeviceModel.hasDevices && count > 0) ? 0 : -1

            delegate: ItemDelegate {
                width: deviceCombo.width
                text: model.displayName
                highlighted: deviceCombo.highlightedIndex === index
            }

            // Show placeholder text when no devices
            Text {
                anchors.left: parent.left
                anchors.leftMargin: deviceCombo.leftPadding
                anchors.verticalCenter: parent.verticalCenter
                text: i18n("Please plug in a USB drive")
                color: Kirigami.Theme.disabledTextColor
                visible: !usbDeviceModel || !usbDeviceModel.hasDevices || deviceCombo.count === 0
            }
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("All data on the selected device will be permanently erased!")
            visible: deviceCombo.currentIndex !== -1 && !flashController.isWriting
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Progress")
            visible: flashController.isWriting || flashController.progress > 0
        }

        Label {
            Layout.fillWidth: true
            wrapMode: Label.WordWrap
            text: flashController.statusMessage
            visible: flashController.isWriting
        }

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: flashController.errorMessage
            visible: flashController.errorMessage
        }

        Kirigami.InlineMessage {
            id: successMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("The ISO has been successfully written to the USB drive.")
            visible: false // Only show on success
        }
    }

    footer: ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Button {
                text: i18n("Start Flashing")
                icon.name: "media-flash"
                highlighted: true
                enabled: isoPath && deviceCombo.currentIndex !== -1 && !flashController.isWriting
                onClicked: {
                    if (deviceCombo.currentIndex < 0 || !usbDeviceModel) {
                        console.error("FlashPage: Invalid device selection");
                        return;
                    }

                    let device = usbDeviceModel.getDevice(deviceCombo.currentIndex);
                    if (device && device.physicalDevice) {
                        console.log("FlashPage: Starting flash with device:", device.physicalDevice);
                        flashController.startFlashing(isoPath, device);
                    } else {
                        console.error("FlashPage: No valid device selected");
                    }
                }
                visible: !flashController.isWriting
            }

            Button {
                text: i18nc("@action:button", "Home")
                icon.name: "go-home"
                onClicked: {
                    // Simply pop back to the first page instead of clearing the stack
                    pageStack.pop(null); // Pop to the first page (WelcomePage)
                }
                visible: !flashController.isWriting
            }
        }
    }
}
