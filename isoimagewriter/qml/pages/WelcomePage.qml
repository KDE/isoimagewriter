import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15
import "../components"

Kirigami.Page {
    id: welcomePage
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
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.05)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }
    }

    DropArea {
        id: dropArea
        anchors.fill: parent

        onEntered: function (drag) {
            if (drag.hasUrls) {
                var hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'));
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }

        onDropped: function (drop) {
            var isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'));
            if (isoFile) {
                selectedFile = isoFile.toString().replace("file://", "");
                hasValidFile = true;
            }
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Kirigami.Units.gridUnit * 3
        width: Math.min(480, parent.width - Kirigami.Units.gridUnit * 4)

        // App icon and title
        WelcomeHeader {
            Layout.alignment: Qt.AlignHCenter
            isDragActive: welcomePage.isDragActive
            hasValidFile: welcomePage.hasValidFile
            selectedFileName: welcomePage.selectedFile.split('/').pop()
        }

        // Action cards
        WelcomeActions {
            Layout.fillWidth: true
            isDragActive: welcomePage.isDragActive
            hasValidFile: welcomePage.hasValidFile
            selectedFile: welcomePage.selectedFile
        }
    }

    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Kirigami.Units.gridUnit * 2
        text: "About"
        flat: true
        onClicked: pageStack.push("qrc:/qml/pages/AboutPage.qml")
    }
}
