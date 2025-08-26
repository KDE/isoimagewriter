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
    id: downloadWriteOptionsPage
    title: i18n("Select USB")

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        // Row 1: ISO Information
        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Label {
                    text: i18n("ISO to download:")
                    font.bold: true
                }

                Label {
                    Layout.fillWidth: true
                    text: isoName || i18n("Unknown ISO")
                    elide: Label.ElideMiddle
                    font.weight: Font.Medium
                    wrapMode: Label.WordWrap
                }
            }
        }

        // Row 2: USB Drive Selection
        Kirigami.AbstractCard {
            Layout.fillWidth: true

            contentItem: ColumnLayout {
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
                    enabled: usbDeviceModel && usbDeviceModel.hasDevices

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
                    text: i18n("⚠️ All data on the selected device will be permanently erased!")
                    color: Kirigami.Theme.negativeTextColor
                    wrapMode: Label.WordWrap
                    visible: usbDeviceCombo.currentIndex >= 0
                    font.weight: Font.Medium
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
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                onClicked: {
                    applicationWindow().pageStack.pop();
                }
            }

            Button {
                id: nextButton
                text: i18n("Next")
                icon.name: "go-next"
                highlighted: true
                enabled: usbDeviceCombo.currentIndex >= 0
                onClicked: {
                    let device = usbDeviceModel.getDevice(usbDeviceCombo.currentIndex);
                    if (device) {
                        applicationWindow().pageStack.push("qrc:/qml/pages/DownloadingPage.qml", {
                            "isoName": isoName,
                            "isoUrl": isoUrl,
                            "isoHash": isoHash,
                            "isoHashAlgo": isoHashAlgo,
                            "selectedDevice": device
                        });
                    }
                }
            }
        }
    }
}
