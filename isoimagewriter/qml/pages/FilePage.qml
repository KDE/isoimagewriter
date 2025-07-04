import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts

Kirigami.Page {
    id: filePage
    title: "Write ISO Image"
    property string preselectedFile: ""

    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            
            Button {
                text: "Back"
                icon.name: "go-previous"
                onClicked: pageStack.pop()
            }
        }
    }

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

    Kirigami.FormLayout {
        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing
        }
        
        // ISO Image Selection
        RowLayout {
            Kirigami.FormData.label: "Write this ISO image:"
            spacing: Kirigami.Units.smallSpacing
            
            TextField {
                id: isoField
                Layout.fillWidth: true
                placeholderText: "Path to ISO image..."
                readOnly: false
            }
            
            Button {
                icon.name: "folder-open"
                text: "Browse"
                onClicked: fileDialog.open()
            }
        }

        // USB Drive Selection
        ComboBox {
            id: usbDriveCombo
            Kirigami.FormData.label: "To this USB drive:"
            Layout.fillWidth: true
            model: ["Please plug in a USB drive"]
            currentIndex: 0
            enabled: false
        }

        // Buttons row
        Item {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            height: 50  // Fixed height for the button row
            
            
            Button {
                id: createButton
                anchors {
                    right: parent.right
                    bottom: parent.bottom
                }
                text: "Create"
                highlighted: true
                enabled: isoField.text.length > 0 && usbDriveCombo.currentIndex > 0
                
                onClicked: {
                    // Handle creation logic here
                    console.log("Creating ISO on USB drive...")
                }
            }
        }
    }
}