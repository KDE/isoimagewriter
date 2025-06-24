import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root
    spacing: Kirigami.Units.gridUnit
    opacity: isDragActive ? 0.3 : 1.0

    property bool isDragActive: false
    property bool hasValidFile: false
    property string selectedFile: ""

    Behavior on opacity {
        OpacityAnimator {
            duration: 250
        }
    }

    // Select file card
    ActionCard {
        Layout.fillWidth: true
        title: hasValidFile ? "Use selected file" : "Select ISO from computer"
        icon: "document-open"
        isActive: hasValidFile

        onClicked: {
            if (hasValidFile) {
                var page = pageStack.push("qrc:/qml/pages/FilePage.qml");
                page.preselectedFile = selectedFile;
            } else {
                pageStack.push("qrc:/qml/pages/FilePage.qml");
            }
        }
    }

    // Download card
    ActionCard {
        Layout.fillWidth: true
        title: "Download from official mirrors"
        icon: "download"

        onClicked: pageStack.push("qrc:/qml/pages/DownloadPage.qml")
    }
}
