import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts 1.15

import org.kde.example 1.0

Kirigami.ApplicationWindow {
    id: root
    title: "ISO Image Writer"
    width: 800
    height: 600
    minimumWidth: 700
    minimumHeight: 500

    pageStack.defaultColumnWidth: root.width

    Component {
        id: welcomePage
        Kirigami.Page {
            title: ""

            property bool isDragActive: dropArea.containsDrag
            property bool hasValidFile: false
            property string selectedFile: ""

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor

                // Subtle gradient overlay
                Rectangle {
                    anchors.fill: parent
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.05) }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }
            }

            DropArea {
                id: dropArea
                anchors.fill: parent

                onEntered: function(drag) {
                    if (drag.hasUrls) {
                        var hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'))
                        hasIso ? drag.accept(Qt.CopyAction) : drag.reject()
                    }
                }

                onDropped: function(drop) {
                    var isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'))
                    if (isoFile) {
                        selectedFile = isoFile.toString().replace("file://", "")
                        hasValidFile = true
                    }
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Kirigami.Units.gridUnit * 3
                width: Math.min(480, parent.width - Kirigami.Units.gridUnit * 4)

                // App icon and title
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: Kirigami.Units.gridUnit * 2

                    Rectangle {
                        Layout.alignment: Qt.AlignHCenter
                        width: 120
                        height: 120
                        radius: 60
                        color: isDragActive ? Kirigami.Theme.highlightColor : Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
                        border.width: isDragActive ? 3 : 0
                        border.color: Kirigami.Theme.highlightColor

                        Behavior on color { ColorAnimation { duration: 250 } }
                        Behavior on border.width { NumberAnimation { duration: 250 } }

                        Kirigami.Icon {
                            anchors.centerIn: parent
                            width: 64
                            height: 64
                            source: "media-optical"
                            color: isDragActive ? "white" : Kirigami.Theme.highlightColor

                            Behavior on color { ColorAnimation { duration: 250 } }
                        }
                    }

                    Kirigami.Heading {
                        Layout.alignment: Qt.AlignHCenter
                        text: isDragActive ? "Drop your ISO here" : "ISO Image Writer"
                        level: 1
                        font.weight: Font.Light
                        color: isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                        Behavior on color { ColorAnimation { duration: 250 } }
                    }

                    Text {
                        Layout.alignment: Qt.AlignHCenter
                        text: hasValidFile ? selectedFile.split('/').pop() : "Create bootable USB drives with ease"
                        color: hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                        font.pixelSize: 14
                        opacity: isDragActive ? 0.7 : 1.0

                        Behavior on opacity { OpacityAnimator { duration: 250 } }
                    }
                }

                // Action cards
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.gridUnit
                    opacity: isDragActive ? 0.3 : 1.0

                    Behavior on opacity { OpacityAnimator { duration: 250 } }

                    // Select file card
                    Rectangle {
                        Layout.fillWidth: true
                        height: 70
                        radius: 12
                        color: hasValidFile ? Qt.rgba(Kirigami.Theme.positiveTextColor.r, Kirigami.Theme.positiveTextColor.g, Kirigami.Theme.positiveTextColor.b, 0.1) : Kirigami.Theme.backgroundColor
                        border.width: 1
                        border.color: hasValidFile ? Kirigami.Theme.positiveTextColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onEntered: parent.color = Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
                            onExited: parent.color = hasValidFile ? Qt.rgba(Kirigami.Theme.positiveTextColor.r, Kirigami.Theme.positiveTextColor.g, Kirigami.Theme.positiveTextColor.b, 0.1) : Kirigami.Theme.backgroundColor

                            onClicked: {
                                if (hasValidFile) {
                                    var page = pageStack.push(filePage)
                                    page.preselectedFile = selectedFile
                                } else {
                                    pageStack.push(filePage)
                                }
                            }
                        }

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: Kirigami.Units.gridUnit

                            Kirigami.Icon {
                                width: 24
                                height: 24
                                source: "document-open"
                                color: Kirigami.Theme.textColor
                            }

                            Text {
                                text: hasValidFile ? "Use selected file" : "Select ISO from computer"
                                font.pixelSize: 16
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }

                    // Download card
                    Rectangle {
                        Layout.fillWidth: true
                        height: 70
                        radius: 12
                        color: Kirigami.Theme.backgroundColor
                        border.width: 1
                        border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor

                            onEntered: parent.color = Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
                            onExited: parent.color = Kirigami.Theme.backgroundColor
                            onClicked: pageStack.push(downloadPage)
                        }

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: Kirigami.Units.gridUnit

                            Kirigami.Icon {
                                width: 24
                                height: 24
                                source: "download"
                                color: Kirigami.Theme.textColor
                            }

                            Text {
                                text: "Download from official mirrors"
                                font.pixelSize: 16
                                color: Kirigami.Theme.textColor
                            }
                        }
                    }
                }
            }

            Button {
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.margins: Kirigami.Units.gridUnit * 2
                text: "About"
                flat: true
                onClicked: pageStack.push(aboutPage)
            }
        }
    }

    Component {
        id: filePage
        Kirigami.Page {
            title: "Select Files"

            property string preselectedFile: ""

            FileDialog {
                id: fileDialog
                title: "Select ISO image"
                nameFilters: ["ISO files (*.iso)", "All files (*)"]
                onAccepted: {
                    isoField.text = fileDialog.fileUrl.toString().replace("file://", "")
                }
            }

            Component.onCompleted: {
                if (preselectedFile) {
                    isoField.text = preselectedFile
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                spacing: Kirigami.Units.gridUnit * 2
                width: Math.min(600, parent.width - Kirigami.Units.gridUnit * 4)

                // File selection card
                Rectangle {
                    Layout.fillWidth: true
                    height: 120
                    radius: 16
                    color: Kirigami.Theme.backgroundColor
                    border.width: 1
                    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.gridUnit * 2
                        spacing: Kirigami.Units.gridUnit

                        Text {
                            text: "ISO Image"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: Kirigami.Theme.textColor
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            TextField {
                                id: isoField
                                Layout.fillWidth: true
                                placeholderText: "No file selected"
                                readOnly: true
                                background: Rectangle {
                                    color: "transparent"
                                    border.width: 0
                                }
                            }

                            Button {
                                text: "Browse"
                                highlighted: true
                                onClicked: fileDialog.open()
                            }
                        }
                    }
                }

                // USB drive card
                Rectangle {
                    Layout.fillWidth: true
                    height: 120
                    radius: 16
                    color: Kirigami.Theme.backgroundColor
                    border.width: 1
                    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.gridUnit * 2
                        spacing: Kirigami.Units.gridUnit

                        Text {
                            text: "USB Drive"
                            font.pixelSize: 14
                            font.weight: Font.Medium
                            color: Kirigami.Theme.textColor
                        }

                        ComboBox {
                            Layout.fillWidth: true
                            model: ["No USB drives detected"]
                            enabled: false

                            background: Rectangle {
                                color: "transparent"
                                border.width: 0
                            }
                        }
                    }
                }

                // Warning message
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    radius: 12
                    color: Qt.rgba(Kirigami.Theme.neutralTextColor.r, Kirigami.Theme.neutralTextColor.g, Kirigami.Theme.neutralTextColor.b, 0.1)
                    border.width: 1
                    border.color: Qt.rgba(Kirigami.Theme.neutralTextColor.r, Kirigami.Theme.neutralTextColor.g, Kirigami.Theme.neutralTextColor.b, 0.3)

                    RowLayout {
                        anchors.centerIn: parent
                        spacing: Kirigami.Units.gridUnit

                        Kirigami.Icon {
                            width: 20
                            height: 20
                            source: "dialog-warning"
                            color: Kirigami.Theme.neutralTextColor
                        }

                        Text {
                            text: "All data on the USB drive will be permanently erased"
                            color: Kirigami.Theme.neutralTextColor
                            font.pixelSize: 14
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
                    text: "Write to USB"
                    highlighted: true
                    enabled: isoField.text.length > 0
                }
            }
        }
    }

    Component {
        id: downloadPage
        Kirigami.Page {
            title: "Download OS"

            property var distributions: [
                { name: "Ubuntu", desc: "Popular, user-friendly Linux distribution" },
                { name: "Kubuntu", desc: "Ubuntu with KDE Plasma desktop" },
                { name: "Fedora", desc: "Cutting-edge features and technologies" },
                { name: "openSUSE", desc: "Stable and reliable Linux distribution" },
                { name: "Debian", desc: "Universal operating system" },
                { name: "Linux Mint", desc: "Elegant and easy to use" },
                { name: "Elementary OS", desc: "Fast, open, and privacy-respecting" },
                { name: "Pop!_OS", desc: "Optimized for developers and creators" },
                { name: "Manjaro", desc: "User-friendly Arch Linux derivative" }
            ]

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Kirigami.Units.gridUnit * 2
                spacing: Kirigami.Units.gridUnit * 2

                // Search field
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    radius: 16
                    color: Kirigami.Theme.backgroundColor
                    border.width: 1
                    border.color: searchField.activeFocus ? Kirigami.Theme.highlightColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                    Behavior on border.color { ColorAnimation { duration: 200 } }

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: Kirigami.Units.gridUnit
                        spacing: Kirigami.Units.gridUnit

                        Kirigami.Icon {
                            width: 20
                            height: 20
                            source: "search"
                            color: Kirigami.Theme.disabledTextColor
                        }

                        TextField {
                            id: searchField
                            Layout.fillWidth: true
                            placeholderText: "Search for an operating system..."
                            font.pixelSize: 16

                            background: Rectangle {
                                color: "transparent"
                                border.width: 0
                            }
                        }
                    }
                }

                // OS list
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    ListView {
                        model: {
                            if (searchField.text.length === 0) return distributions
                            return distributions.filter(item =>
                                item.name.toLowerCase().includes(searchField.text.toLowerCase())
                            )
                        }

                        spacing: Kirigami.Units.gridUnit

                        delegate: Rectangle {
                            width: ListView.view.width
                            height: 80
                            radius: 12
                            color: Kirigami.Theme.backgroundColor
                            border.width: 1
                            border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: Qt.PointingHandCursor

                                onEntered: parent.color = Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
                                onExited: parent.color = Kirigami.Theme.backgroundColor

                                onClicked: {
                                    searchField.text = modelData.name
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: Kirigami.Units.gridUnit * 1.5
                                spacing: Kirigami.Units.gridUnit

                                Rectangle {
                                    width: 40
                                    height: 40
                                    radius: 20
                                    color: Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.2)

                                    Text {
                                        anchors.centerIn: parent
                                        text: modelData.name.charAt(0)
                                        font.pixelSize: 18
                                        font.weight: Font.Bold
                                        color: Kirigami.Theme.highlightColor
                                    }
                                }

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 4

                                    Text {
                                        text: modelData.name + " (x86_64)"
                                        font.pixelSize: 16
                                        font.weight: Font.Medium
                                        color: Kirigami.Theme.textColor
                                    }

                                    Text {
                                        text: modelData.desc
                                        font.pixelSize: 12
                                        color: Kirigami.Theme.disabledTextColor
                                    }
                                }

                                Kirigami.Icon {
                                    width: 16
                                    height: 16
                                    source: "arrow-right"
                                    color: Kirigami.Theme.disabledTextColor
                                }
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
                    enabled: searchField.text.length > 0
                }
            }
        }
    }

    Component {
        id: aboutPage
        Kirigami.AboutPage {
            aboutData: About
        }
    }

    pageStack.initialPage: welcomePage
}
