/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

Kirigami.ScrollablePage {
    title: i18n("Download OS")

    property string searchText: ""
    property string selectedOS: ""

    readonly property var osList: ["Ubuntu", "Fedora", "Debian", "openSUSE", "Manjaro", "Kubuntu", "Garuda"]
    readonly property var filteredOS: osList.filter(os => searchText === "" || os.toLowerCase().includes(searchText.toLowerCase()))

    actions: [
        Kirigami.Action {
            displayComponent: Kirigami.SearchField {
                onTextChanged: searchText = text
            }
        }
    ]

    ListView {
        model: filteredOS

        delegate: RadioDelegate {
            width: ListView.view.width
            text: modelData
            checked: selectedOS === modelData
            onClicked: selectedOS = modelData
            icon.name: "media-optical"
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: parent.count === 0
            text: i18n("No operating systems found")
            icon.name: "search"
        }
    }
}
