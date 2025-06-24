import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
import "../components"

Kirigami.Page {
    id: filePage
    title: "Select Files"

    property string preselectedFile: ""

    FileDialog {
        id: fileDialog
        title: "Select ISO image"
        nameFilters: ["ISO files (*.iso)", "All files (*)"]
        onAccepted: {
            isoField.text = fileDialog.fileUrl.toString().replace("file://", "");
        }
    }

    Component.onCompleted: {
        if (preselectedFile) {
            isoField.text = preselectedFile;
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        spacing: Kirigami.Units.gridUnit * 2
        width: Math.min(600, parent.width - Kirigami.Units.gridUnit * 4)

        // File selection card
        FileSelectionCard {
            id: fileSelectionCard
            Layout.fillWidth: true
            title: "ISO Image"
            fieldText: isoField.text
            onBrowseClicked: fileDialog.open()

            TextField {
                id: isoField
                placeholderText: "No file selected"
                readOnly: true
                background: Rectangle {
                    color: "transparent"
                    border.width: 0
                }
            }
        }

        // USB drive card
        UsbDriveCard {
            Layout.fillWidth: true
        }

        // Warning message
        WarningCard {
            Layout.fillWidth: true
            message: "All data on the USB drive will be permanently erased"
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
