/*
* SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
* SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtQuick.Controls as Controls
import QtQuick.Layouts 
import org.kde.kirigami as Kirigami
import org.kde.isoimagewriter

Kirigami.Page {
    id: flashPage
    title: flashCompleted ? i18nc("@title:window", "USB Drive Ready") : flashFailed ? i18nc("@title:window", "Flash Failed") : i18nc("@title:window", "Creating USB Drive")
    
    // Disable header navigation during flashing
    globalToolBarStyle: (flashingStarted && !flashCompleted && !flashFailed) ? 
                       Kirigami.ApplicationHeaderStyle.None : 
                       Kirigami.ApplicationHeaderStyle.Auto

    property string isoPath: ""
    property var selectedDevice: null
    property bool flashingStarted: false
    property bool flashCompleted: false
    property bool flashFailed: false
    property string errorMessage: ""
    property real currentProgress: 0.0

    Component.onCompleted: {
        console.log("FlashPage: Ready to flash ISO");
        // Auto-start flashing when page loads
        Qt.callLater(startFlashing);
    }

    function startFlashing() {
        if (selectedDevice && isoPath && !flashingStarted) {
            flashingStarted = true;
            console.log("FlashPage: Starting flash with device:", selectedDevice.physicalDevice);
            console.log("FlashPage: Progress bar should now be visible:", flashingStarted && !flashCompleted && !flashFailed);
            flashController.startFlashing(isoPath, selectedDevice);
        }
    }

    FlashController {
        id: flashController

        onProgressChanged: {
            console.log("FlashPage: Progress changed to", progress);
            flashPage.currentProgress = progress;
        }

        onFlashCompleted: {
            console.log("FlashPage: Flash completed");
            flashPage.flashCompleted = true;
            flashPage.flashFailed = false;
        }

        onFlashFailed: function (error) {
            console.log("FlashPage: Flash failed with error:", error);
            flashPage.flashCompleted = false;
            flashPage.flashFailed = true;
            flashPage.errorMessage = error;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 40)
        spacing: Kirigami.Units.largeSpacing * 2

        // Success/Error Icon
        Kirigami.Icon {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            source: flashCompleted ? "checkmark" : flashFailed ? "error" : "media-optical"
            color: flashCompleted ? Kirigami.Theme.positiveTextColor : flashFailed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            visible: flashCompleted || flashFailed || !flashingStarted
        }

        // Main status message
        Controls.Label {
            Layout.fillWidth: true
            text: flashCompleted ? i18nc("@info:status", "✅ USB Drive Ready!") : flashFailed ? i18nc("@info:status", "❌ Flash Failed") : i18nc("@info:progress", "Creating your bootable USB drive")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.6
            font.bold: flashCompleted || flashFailed
            color: flashCompleted ? Kirigami.Theme.positiveTextColor : flashFailed ? Kirigami.Theme.negativeTextColor : Kirigami.Theme.textColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Subtitle message
        Controls.Label {
            Layout.fillWidth: true
            text: flashCompleted ? i18nc("@info", "Your bootable USB drive has been created successfully!\nYou can now safely remove it and use it to boot your computer.") : flashFailed ? (errorMessage || i18nc("@info", "Please try again or check your USB drive.")) : i18nc("@info", "This may take some time depending on the size of the ISO image file and the transfer speed.")
            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.1
            color: flashCompleted ? Kirigami.Theme.textColor : Kirigami.Theme.disabledTextColor
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WordWrap
        }

        // Progress bar with percentage
        ColumnLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing
            visible: flashingStarted && !flashCompleted && !flashFailed
            
            Component.onCompleted: {
                console.log("FlashPage: Progress bar container created, visible:", visible);
            }
            
            onVisibleChanged: {
                console.log("FlashPage: Progress bar visibility changed to:", visible, 
                           "flashingStarted:", flashingStarted, 
                           "flashCompleted:", flashCompleted, 
                           "flashFailed:", flashFailed);
            }

            // Progress bar container with overlay text
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                Controls.ProgressBar {
                    id: flashProgressBar
                    anchors.fill: parent
                    from: 0
                    to: 1
                    value: currentProgress
                    
                    onValueChanged: {
                        console.log("FlashPage: ProgressBar value changed to:", value);
                    }
                }

                // Progress percentage text overlay
                Controls.Label {
                    anchors.centerIn: parent
                    text: Math.round(currentProgress * 100) + "%"
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
                    font.bold: true
                    color: Kirigami.Theme.textColor
                }
            }

            Controls.Label {
                Layout.fillWidth: true
                text: i18nc("@info:progress", "Writing data to USB drive…")
                horizontalAlignment: Text.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.9
            }
        }

        // Error actions (only show Retry button in main content when failed)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.mediumSpacing
            visible: flashFailed

            Controls.Button {
                text: i18nc("@action:button", "Retry")
                icon.name: "view-refresh"
                highlighted: true
                onClicked: {
                    flashingStarted = false;
                    flashCompleted = false;
                    flashFailed = false;
                    errorMessage = "";
                    currentProgress = 0.0;
                    Qt.callLater(startFlashing);
                }
            }
        }
    }

    // Footer with Cancel/Done button
    footer: Controls.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

            Controls.Button {
                text: i18nc("@action:button", "Cancel")
                icon.name: "dialog-cancel"
                visible: !flashCompleted && !flashFailed
                onClicked: {
                    if (flashingStarted && !flashCompleted && !flashFailed) {
                        flashController.cancelFlashing();
                    }
                    applicationWindow().pageStack.pop();
                }
            }

            Controls.Button {
                text: i18nc("@action:button", "Done")
                icon.name: "dialog-ok"
                highlighted: true
                visible: flashCompleted
                onClicked: {
                    Qt.quit();
                }
            }
        }
    }
}
