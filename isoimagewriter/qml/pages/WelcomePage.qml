import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Layouts
import "../components"

Kirigami.Page {
    id: welcomePage
    title: ""

    property bool isDragActive: dropArea.containsDrag
    property bool hasValidFile: false
    property string selectedFile: ""
    readonly property bool networkConnected:false

    DropArea {
        id: dropArea
        anchors.fill: parent

        onEntered: function(drag) {
            if (drag.hasUrls) {
                var hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'));
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }

        onDropped: function(drop) {
            var isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'));
            if (isoFile) {
                welcomePage.selectedFile = isoFile.toString().replace("file://", "");
                welcomePage.hasValidFile = true;
            }
        }
    }

    ColumnLayout {
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Kirigami.Units.gridUnit
        }
        spacing: Kirigami.Units.gridUnit

        Row {
            spacing: Kirigami.Units.gridUnit * 2
            width: parent.width

            Rectangle {
                id: iconContainer
                width: 64
                height: 64
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

            // Text content on the right
            Column {
                width: parent.width - iconContainer.width - Kirigami.Units.gridUnit * 2
                spacing: Kirigami.Units.smallSpacing

                Text {
                    width: parent.width
                    text: welcomePage.isDragActive ? "Drop your ISO here" : "KDE ISO Image Writer"
                    color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }

                Text {
                    width: parent.width
                    text: welcomePage.hasValidFile
                          ? welcomePage.selectedFile.split('/').pop()
                          : "A quick and simple way to create bootable USB drives"
                    color: welcomePage.hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                    opacity: welcomePage.isDragActive ? 0.7 : 1.0
                }
            }
        }


            Row {
                width: parent.width
                spacing: Kirigami.Units.gridUnit
                
                Button {
                    height: 70
                    text: welcomePage.hasValidFile ? "Use selected file" : "Select ISO from computer"
                    icon.name: "document-open"
                    
                    onClicked: {
                        if (welcomePage.hasValidFile) {
                            var page = pageStack.push("qrc:/qml/pages/FilePage.qml");
                            page.preselectedFile = welcomePage.selectedFile;
                        } else {
                            pageStack.push("qrc:/qml/pages/FilePage.qml");
                        }
                    }
                }

                // Download button
                Button {
                    height: 70
                    text: "Download from official mirrors"
                    icon.name: "download"
                    
                    onClicked: pageStack.push("qrc:/qml/pages/DownloadPage.qml")
                }
            }
        }
    

    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "About"
        flat: true
        onClicked: pageStack.push("qrc:/qml/pages/AboutPage.qml")
    }
}

