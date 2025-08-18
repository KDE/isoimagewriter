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
    title: i18n("Download Directly")

    property string searchText: ""
    property string selectedOS: ""

    ReleasesModel {
        id: releasesModel
    }

    actions: [
        Kirigami.Action {
            displayComponent: Kirigami.SearchField {
                onTextChanged: searchText = text
            }
        }
    ]

    ListView {
        model: releasesModel

        delegate: Item {
            width: ListView.view.width
            height: radioDelegate.height

            // Only show items that match the search filter
            visible: searchText === "" || model.name.toLowerCase().includes(searchText.toLowerCase())

            RadioDelegate {
                id: radioDelegate
                anchors.left: parent.left
                anchors.right: parent.right
                text: model.name
                checked: selectedOS === model.name
                onClicked: selectedOS = model.name
                icon.name: "media-optical"

                ToolTip {
                    visible: parent.hovered
                    text: model.description + "\n\nVersion: " + model.version + (model.edition ? " " + model.edition : "") + "\nURL: " + model.url
                    // change to standard duration
                    delay: 500
                    timeout: 5000
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: parent.count === 0
            text: i18n("No operating systems found")
            icon.name: "search"
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            

            Button {
                text: i18n("Download")
                icon.name: "download"
                enabled: selectedOS !== ""
                onClicked: {
                    // Find the selected release
                    for (let i = 0; i < releasesModel.rowCount(); i++) {
                        let release = releasesModel.getReleaseAt(i);
                        if (release.name === selectedOS) {
                            applicationWindow().pageStack.push("qrc:/qml/pages/DownloadingPage.qml", {
                                "isoName": release.name,
                                "isoUrl": release.url,
                                "isoHash": release.hash,
                                "isoHashAlgo": release.hashAlgo
                            });
                            break;
                        }
                    }
                }
            }
        }
    }
}
