// Automatic redirection to SelectionPage when ISO is dropped - implemented

/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    id: welcomePage

    property bool isDragActive: dropArea.containsDrag
    property bool hasValidFile: false
    property string selectedFile: ""
    readonly property bool networkConnected: false
    
    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "About")
            icon.name: "help-about"
            onTriggered: pageStack.push("qrc:/qml/pages/AboutPage.qml")
        }
    ]

    DropArea {
        id: dropArea

        anchors.fill: parent
        onEntered: function(drag) {
            if (drag.hasUrls) {
                let hasIso = drag.urls.some((url) => {
                    return url.toString().toLowerCase().endsWith('.iso');
                });
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }
        onDropped: function(drop) {
            let isoFile = drop.urls.find((url) => {
                return url.toString().toLowerCase().endsWith('.iso');
            });
            if (isoFile) {
                let filePath = isoFile.toString().replace("file://", "");
                welcomePage.selectedFile = filePath;
                welcomePage.hasValidFile = true;
                // Auto-navigate to SelectionPage with the dropped ISO
                console.log("WelcomePage: Dropping ISO file:", filePath);
                Qt.callLater(function() {
                    console.log("WelcomePage: Pushing SelectionPage with preselectedFile:", filePath);
                    let page = pageStack.push("qrc:/qml/pages/SelectionPage.qml", {
                        "preselectedFile": filePath
                    });
                    console.log("WelcomePage: Page created with preselectedFile:", page.preselectedFile);
                });
            }
        }
    }

    // Main content area
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(welcomePage.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 35)
        spacing: Kirigami.Units.largeSpacing * 1.5

        // Header section with icon and title
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing

            Item {
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge

                Kirigami.Icon {
                    anchors.fill: parent
                    source: welcomePage.isDragActive ? "document-import" : "qrc:/qml/images/org.kde.isoimagewriter.svg"
                }

            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: welcomePage.isDragActive ? i18nc("@info", "Drop to select ISO file") : i18nc("@title", "KDE ISO Image Writer")
                    color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    level: 1
                    font.weight: Font.Bold

                    Behavior on color {
                        ColorAnimation {
                            duration: Kirigami.Units.shortDuration
                        }

                    }

                }

                Label {
                    Layout.fillWidth: true
                    text: welcomePage.isDragActive ? i18nc("@info", "Release to continue with this ISO file") : welcomePage.hasValidFile ? i18nc("@info:status", "Ready to write: %1", welcomePage.selectedFile.split('/').pop()) : i18nc("@info", "A quick and simple way to create bootable USB drives. Drag and drop an ISO file to get started.")
                    color: welcomePage.hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.95
                }

            }

        }

        // Action buttons section
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            // Primary action - Download ISOs
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                text: i18nc("@action:button", "Browse Linux Distributions")
                icon.name: "download"
                font.bold: true
                highlighted: true
                ToolTip.text: i18nc("@info:tooltip", "Download popular Linux distributions like Kubuntu, Fedora, and more")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                onClicked: pageStack.push("qrc:/qml/pages/IsoListingPage.qml")
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                text: welcomePage.hasValidFile ? i18nc("@action:button", "Use Selected ISO File") : i18nc("@action:button", "Choose Local ISO File")
                icon.name: welcomePage.hasValidFile ? "media-optical" : "document-open"
                ToolTip.text: i18nc("@info:tooltip", "Select an ISO file from your computer")
                ToolTip.visible: hovered
                ToolTip.delay: Kirigami.Units.toolTipDelay
                onClicked: {
                    if (welcomePage.hasValidFile)
                        pageStack.push("qrc:/qml/pages/SelectionPage.qml", {
                            "preselectedFile": welcomePage.selectedFile
                        });
                    else
                        pageStack.push("qrc:/qml/pages/SelectionPage.qml");
                }
            }

        }

    }



}
