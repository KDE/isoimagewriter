import QtQuick
import QtQuick.Controls 
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

import org.kde.isoimagewriter 1.0

Kirigami.ScrollablePage {
    id: flashPage
    title: i18n("Write to USB Drive")

    property string isoPath: "/path/to/your/selected/image.iso"

    // Real FlashController instance from C++
    FlashController {
        id: flashController
        
        onFlashCompleted: {
            successMessage.visible = true;
        }
        
        onFlashFailed: function(error) {
            console.error("Flash failed:", error);
        }
    }

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Information")
        }

        Label {
            Kirigami.FormData.label: i18n("Source ISO:")
            text: isoPath.split('/').pop() || i18n("No file selected")
            elide: Text.ElideMiddle
        }


        ComboBox {
            id: deviceCombo
            Kirigami.FormData.label: i18n("Target Device:")
            Layout.fillWidth: true

            // Bind the model directly to the C++ UsbDeviceModel instance
            model: usbDeviceModel

            // Use the 'displayName' role for the text in the dropdown
            textRole: "displayName"

            // The ComboBox is enabled only if there are devices and we are not writing
            enabled: usbDeviceModel.hasDevices && !flashController.isWriting

            // Show appropriate text when no devices are available
            displayText: usbDeviceModel.hasDevices ? currentText : i18n("Please plug in a USB drive")
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("All data on the selected device will be permanently erased!")
            visible: deviceCombo.currentIndex !== -1 && !flashController.isWriting
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Progress")
            visible: flashController.isWriting || flashController.progress > 0
        }


        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: flashController.statusMessage
            visible: flashController.isWriting
        }

        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: flashController.errorMessage
            visible: flashController.errorMessage
        }

        Kirigami.InlineMessage {
            id: successMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("The ISO has been successfully written to the USB drive.")
            visible: false // Only show on success
        }
    }

    footer: ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }

          Button {
            text: i18n("Start Flashing")
            icon.name: "media-flash"
            highlighted: true
            enabled: isoPath && deviceCombo.currentIndex !== -1 && !flashController.isWriting
            onClicked: {
              let device = usbDeviceModel.getDevice(deviceCombo.currentIndex);
              if (device) {
                flashController.startFlashing(isoPath, device);
                // progressBar.visible = true
              }
            }
            visible: !flashController.isWriting
          }

          Button {
            text: i18n("Home")
            icon.name: "house"
            highlighted: true
            enabled: false
            onClicked: {
              pageStack.replace("qrc:/qml/pages/WelcomePage")


            }
            visible: !flashController.isWriting
          }
        }
    }
}

