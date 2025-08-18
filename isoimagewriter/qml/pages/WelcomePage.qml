/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

Kirigami.Page {
    id: welcomePage
    title: ""

    property bool isDragActive: dropArea.containsDrag
    property bool hasValidFile: false
    property string selectedFile: ""
    readonly property bool networkConnected: false

    DropArea {
        id: dropArea
        anchors.fill: parent

        onEntered: function (drag) {
            if (drag.hasUrls) {
                let hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'));
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }

        onDropped: function (drop) {
            let isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'));
            if (isoFile) {
                welcomePage.selectedFile = isoFile.toString().replace("file://", "");
                welcomePage.hasValidFile = true;
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(welcomePage.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 40)
        spacing: Kirigami.Units.largeSpacing

        RowLayout {
            Layout.fillWidth: true
            spacing: Kirigami.Units.largeSpacing

            Rectangle {
                id: iconContainer
                Layout.preferredWidth: Kirigami.Units.iconSizes.large
                Layout.preferredHeight: Kirigami.Units.iconSizes.large
                radius: width / 2
                color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.backgroundColor
                border.width: welcomePage.isDragActive ? 0 : 2
                border.color: Kirigami.Theme.highlightColor

                Kirigami.Icon {
                    anchors.centerIn: parent
                    width: Kirigami.Units.iconSizes.large
                    height: Kirigami.Units.iconSizes.large
                    source: welcomePage.isDragActive ? "document-import" : "qrc:/qml/images/org.kde.isoimagewriter.svg"
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: Kirigami.Units.smallSpacing

                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: welcomePage.isDragActive ? i18n("Drop your ISO here") : i18n("KDE ISO Image Writer")
                    color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Label.WordWrap
                    level: 1
                    font.weight: Font.Bold
                }

                Label {
                    Layout.fillWidth: true
                    text: welcomePage.hasValidFile ? welcomePage.selectedFile.split('/').pop() : i18n("A quick and simple way to create bootable USB drives")
                    color: welcomePage.hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                    wrapMode: Label.WordWrap
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                text: welcomePage.hasValidFile ? i18n("Use selected file") : i18n("Select ISO from computer")
                icon.name: "document-open"

                onClicked: {
                    if (welcomePage.hasValidFile) {
                        let page = pageStack.push("qrc:/qml/pages/SelectionPage.qml");
                        page.preselectedFile = welcomePage.selectedFile;
                    } else {
                        pageStack.push("qrc:/qml/pages/SelectionPage.qml");
                    }
                }
            }

            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                text: i18n("Download automatically")
                icon.name: "download"

                onClicked: pageStack.push("qrc:/qml/pages/IsoListingPage.qml")
            }
        }
    }

    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: i18nc("@action:button", "About")
        icon.name: "help-about"
        onClicked: pageStack.push("qrc:/qml/pages/AboutPage.qml")
    }
}
