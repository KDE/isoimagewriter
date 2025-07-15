import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.20 as Kirigami
import QtQuick.Layouts

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
                let hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'));
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }

        onDropped: function(drop) {
            let isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'));
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
                width: Kirigami.Units.iconSizes.large
                height: Kirigami.Units.iconSizes.large
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
                    text: welcomePage.isDragActive ? i18n("Drop your ISO here") : i18n("KDE ISO Image Writer")
                    color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.pixelSize: 24
                    font.weight: Font.Bold
                }

                Text {
                    width: parent.width
                    text: welcomePage.hasValidFile
                          ? welcomePage.selectedFile.split('/').pop()
                          : i18n("A quick and simple way to create bootable USB drives")
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
                    text: welcomePage.hasValidFile ? i18n("Use selected file") : i18n("Select ISO from computer")
                    icon.name: "document-open"
                    
                    onClicked: {
                        if (welcomePage.hasValidFile) {
                            let page = pageStack.push("qrc:/qml/pages/FilePage.qml");
                            page.preselectedFile = welcomePage.selectedFile;
                        } else {
                            pageStack.push("qrc:/qml/pages/FilePage.qml");
                        }
                    }
                }

                // Download button
                Button {
                    height: 70
                    text: i18n("Download automatically")
                    icon.name: "download"
                    
                    onClicked: pageStack.push("qrc:/qml/pages/DownloadPage.qml")
                }
            }
        }
    

    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: i18n("About")
        icon.name: "help-about"
        onClicked: pageStack.push("qrc:/qml/pages/AboutPage.qml")
    }
}

