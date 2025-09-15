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

FormCard.FormCardPage {
    id: downloadWriteOptionsPage
    title: i18nc("@title:window", "Download & Write")

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""
    property var selectedDevice: null

    function navigateToDownloadingPage() {
        if (!selectedDevice || !selectedDevice.physicalDevice) {
            console.error("DownloadWriteOptionsPage: Invalid device selection");
            return;
        }

        console.log("DownloadWriteOptionsPage: Navigating to downloading page with device:", selectedDevice.physicalDevice);
        applicationWindow().pageStack.push("qrc:/qml/pages/DownloadingPage.qml", {
            "isoName": isoName,
            "isoUrl": isoUrl,
            "isoHash": isoHash,
            "isoHashAlgo": isoHashAlgo,
            "selectedDevice": selectedDevice
        });
    }
    UsbDeviceModel {
        id: usbDeviceModel
    }

    // USB Device Selection Dialog
    Kirigami.Dialog {
        id: usbSelectionDialog
        title: i18nc("@title:window", "Select USB Device")
        standardButtons: Kirigami.Dialog.NoButton

        property var selectedDevice: null

        signal deviceSelected(var device)

        onDeviceSelected: function (device) {
            downloadWriteOptionsPage.selectedDevice = device;
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

        // ISO Information Section
        FormCard.FormHeader {
            title: i18nc("@title:group", "ISO to Download")
        }

        FormCard.FormCard {
            RowLayout {
                Layout.fillWidth: true
                Layout.margins: Kirigami.Units.largeSpacing
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Icon {
                    source: "application-x-cd-image"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Controls.Label {
                        Layout.fillWidth: true
                        text: isoName || i18nc("@info", "Unknown ISO")
                        font.bold: true
                        elide: Text.ElideMiddle
                        wrapMode: Text.WordWrap
                    }

                    Controls.Label {
                        Layout.fillWidth: true
                        text: isoUrl || i18nc("@info", "No URL provided")
                        color: Kirigami.Theme.disabledTextColor
                        elide: Text.ElideMiddle
                        wrapMode: Text.WordWrap
                        font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
                    }
                }
            }
        }

        // USB Device Selection Section
        FormCard.FormHeader {
            title: i18nc("@title:group", "USB Device")
        }

        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                id: usbSelectionDelegate
                text: selectedDevice ? selectedDevice.displayName : i18nc("@action:button", "Select USB device...")
                description: selectedDevice ? (selectedDevice.size ? i18nc("@info", "Size: %1", selectedDevice.size) : i18nc("@info", "USB device selected")) : i18nc("@info", "Choose a USB device to write to")
                icon.name: selectedDevice ? "drive-removable-media" : "drive-removable-media-usb"
                enabled: usbDeviceModel && usbDeviceModel.hasDevices
                onClicked: usbSelectionDialog.open()
            }
        }

        // Warning message when no USB devices
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
                    navigateToDownloadingPage();
                }
            }
        ]
    }

    footer: Controls.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                onClicked: {
                    applicationWindow().pageStack.pop();
                }
            }

            Controls.Button {
                text: i18nc("@action:button", "Next")
                icon.name: "go-next"
                highlighted: true
                enabled: selectedDevice !== null
                onClicked: {
                    confirmationDialog.open();
                }
            }
        }
    }
}
