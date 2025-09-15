// SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

Kirigami.Page {
    id: root

    property bool isDragActive: dropArea.containsDrag
    property bool hasValidFile: false
    property string selectedFile: ""
    readonly property bool networkConnected: false

    leftPadding: 0
    rightPadding: 0

    actions: [
        Kirigami.Action {
            text: i18nc("@action:button", "About")
            icon.name: "help-about"
            onTriggered: root.Kirigami.PageStack.push(Qt.createComponent("org.kde.kirigamiaddons.formcard", "AboutPage"))
        }
    ]

    DropArea {
        id: dropArea

        anchors.fill: parent

        onEntered: (drag) => {
            if (drag.hasUrls) {
                const hasIso = drag.urls.some((url) => {
                    return url.toString().toLowerCase().endsWith('.iso');
                });
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }
        onDropped: (drop) => {
            let isoFile = drop.urls.find((url) => {
                return url.toString().toLowerCase().endsWith('.iso');
            });
            if (isoFile) {
                let filePath = isoFile.toString().replace("file://", "");
                root.selectedFile = filePath;
                root.hasValidFile = true;
                // Auto-navigate to SelectionPage with the dropped ISO
                console.log("root: Dropping ISO file:", filePath);
                Qt.callLater(function() {
                    console.log("root: Pushing SelectionPage with preselectedFile:", filePath);
                    let page = root.Kirigami.PageStack.push(Qt.createComponent("org.kde.isoimagewriter", "SelectionPage"), {
                        "preselectedFile": filePath,
                    });
                    console.log("root: Page created with preselectedFile:", page.preselectedFile);
                });
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            spacing: Kirigami.Units.largeSpacing

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumWidth: distributionsCard.maximumWidth - Kirigami.Units.largeSpacing * 2

            Kirigami.Icon {
                source: root.isDragActive ? "document-import" : "org.kde.isoimagewriter"

                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            }

            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter

                Kirigami.Heading {
                    text: root.isDragActive ? i18nc("@info", "Drop to select ISO file") : i18nc("@title", "KDE ISO Image Writer")
                    color: root.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.weight: Font.Bold

                    Layout.fillWidth: true

                    Behavior on color {
                        ColorAnimation {
                            duration: Kirigami.Units.shortDuration
                        }
                    }
                }

                Label {
                    text: if (root.isDragActive) {
                        return i18nc("@info", "Release to continue with this ISO file");
                    } else if (root.hasValidFile) {
                        return i18nc("@info:status", "Ready to write: %1", root.selectedFile.split('/').pop());
                    } else {
                        return i18nc("@info", "A quick and simple way to create bootable USB drives. Drag and drop an ISO file to get started.");
                    }
                    color: root.hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.95

                    Layout.fillWidth: true
                }
            }
        }

        FormCard.FormCard {
            id: distributionsCard

            FormCard.FormButtonDelegate {
                text: i18nc("@action:button", "Browse Linux Distributions")
                icon.name: "download"
                description: i18nc("@info", "Download popular Linux distributions like Kubuntu, Fedora, and more")
                onClicked: root.Kirigami.PageStack.push(Qt.createComponent("org.kde.isoimagewriter", "IsoListingPage"))
            }
        }

        FormCard.FormCard {
            FormCard.FormButtonDelegate {
                text: root.hasValidFile ? i18nc("@action:button", "Use Selected ISO File") : i18nc("@action:button", "Choose Local ISO File")
                icon.name: root.hasValidFile ? "media-optical" : "document-open"
                description: i18nc("@info:tooltip", "Select an ISO file from your computer")
                onClicked: {
                    if (root.hasValidFile) {
                        root.Kirigami.PageStack.push(Qt.createComponent("org.kde.isoimagewriter", "SelectionPage"), {
                            "preselectedFile": root.selectedFile
                        });
                    } else {
                        root.Kirigami.PageStack.push(Qt.createComponent("org.kde.isoimagewriter", "SelectionPage"))
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
