import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15
import "../components"

Kirigami.Page {
    id: downloadPage
    title: "Download OS"

    property var distributions: [
        {
            name: "Ubuntu",
            desc: "Popular, user-friendly Linux distribution"
        },
        {
            name: "Kubuntu",
            desc: "Ubuntu with KDE Plasma desktop"
        },
        {
            name: "Fedora",
            desc: "Cutting-edge features and technologies"
        },
        {
            name: "openSUSE",
            desc: "Stable and reliable Linux distribution"
        },
        {
            name: "Debian",
            desc: "Universal operating system"
        },
        {
            name: "Linux Mint",
            desc: "Elegant and easy to use"
        },
        {
            name: "Elementary OS",
            desc: "Fast, open, and privacy-respecting"
        },
        {
            name: "Pop!_OS",
            desc: "Optimized for developers and creators"
        },
        {
            name: "Manjaro",
            desc: "User-friendly Arch Linux derivative"
        }
    ]

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit * 2
        spacing: Kirigami.Units.gridUnit * 2

        // Search field
        SearchField {
            id: searchField
            Layout.fillWidth: true
        }

        // OS list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                model: {
                    if (searchField.searchText.length === 0)
                        return distributions;
                    return distributions.filter(item => item.name.toLowerCase().includes(searchField.searchText.toLowerCase()));
                }

                spacing: Kirigami.Units.gridUnit

                delegate: DistributionItem {
                    width: ListView.view.width
                    distributionData: modelData

                    onClicked: {
                        searchField.searchText = modelData.name;
                    }
                }
            }
        }
    }

    RowLayout {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Kirigami.Units.gridUnit * 2
        spacing: Kirigami.Units.gridUnit

        Button {
            text: "Back"
            onClicked: pageStack.pop()
        }

        Button {
            text: "Download"
            highlighted: true
            enabled: searchField.searchText.length > 0
        }
    }
}
