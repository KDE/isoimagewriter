import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts

Kirigami.ScrollablePage {
    id: flashPage
    title: i18n("Write to USB Drive")
    
    property string isoPath: ""
    property string devicePath: ""
    property bool isFlashing: false
    property bool flashComplete: false
    
    property var flashWorker: ({ 
        progress: 0,
        status: "",
        error: "",
        startFlashing: function() { console.log("Start flashing"); },
        cancel: function() { console.log("Cancel flashing"); }
    })
    
    // Timer to simulate progress updates 
    Timer {
        id: progressTimer
        interval: 100
        repeat: true
        running: isFlashing && !flashComplete
        onTriggered: {
            if (flashWorker.progress < 100) {
                flashWorker.progress += 1;
                flashWorker.status = i18n("Writing: %1% complete").arg(flashWorker.progress);
            } else {
                flashComplete = true;
                flashWorker.status = i18n("Flash complete!");
                stop();
            }
        }
    }
    
    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing
        
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Information")
        }
        
        RowLayout {
            Kirigami.FormData.label: i18n("Source ISO:")
            spacing: Kirigami.Units.smallSpacing
            
            Label {
                text: {
                    let fileName = isoPath.split('/').pop();
                    return fileName || i18n("No file selected");
                }
                elide: Text.ElideMiddle
                Layout.fillWidth: true
            }
            
            ToolButton {
                icon.name: "document-open"
                onClicked: {
                    // Show file dialog to select ISO
                    console.log("Open file dialog");
                }
                enabled: !isFlashing
            }
        }
        
        ComboBox {
            id: deviceCombo
            Kirigami.FormData.label: i18n("Target Device:")
            Layout.fillWidth: true
            model: [i18n("Select a USB drive"), 
                   i18n("SanDisk Ultra (32GB) - /dev/sdb"), 
                   i18n("Kingston DataTraveler (16GB) - /dev/sdc")]
            enabled: !isFlashing && !flashComplete
            
            onCurrentIndexChanged: {
                if (currentIndex > 0) {
                    devicePath = model[currentIndex];
                }
            }
        }
        
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("All data on the selected device will be permanently erased!")
            showCloseButton: true
            visible: deviceCombo.currentIndex > 0 && !isFlashing && !flashComplete
        }
        
        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18n("Flash Progress")
            visible: isFlashing || flashComplete
        }
        
        ProgressBar {
            id: progressBar
            Layout.fillWidth: true
            value: flashWorker.progress / 100
            visible: isFlashing || flashComplete
        }
        
        Label {
            id: statusLabel
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: isFlashing ? flashWorker.status : 
                             flashComplete ? i18n("Flash completed successfully!") : 
                                           i18n("Ready to flash")
            color: flashWorker.error ? Kirigami.Theme.negativeTextColor : 
                                     flashComplete ? Kirigami.Theme.positiveTextColor : 
                                                   Kirigami.Theme.textColor
            visible: isFlashing || flashComplete || deviceCombo.currentIndex > 0
        }
        
        // Error Message
        Kirigami.InlineMessage {
            id: errorMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: flashWorker.error
            showCloseButton: true
            visible: flashWorker.error
        }
        
        // Success Message
        Kirigami.InlineMessage {
            id: successMessage
            Layout.fillWidth: true
            type: Kirigami.MessageType.Positive
            text: i18n("The ISO has been successfully written to the USB drive.")
            showCloseButton: true
            visible: flashComplete
        }
        
        Item {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            Layout.preferredHeight: buttonRow.height
            
            RowLayout {
                id: buttonRow
                anchors.right: parent.right
                spacing: Kirigami.Units.smallSpacing
                
                Button {
                    id: backButton
                    text: i18n("Back")
                    icon.name: "go-previous"
                    onClicked: {
                        // Go back to previous page
                        pageStack.pop();
                    }
                    visible: !isFlashing
                }
                
                Button {
                    id: cancelButton
                    text: i18n("Cancel")
                    icon.name: "dialog-cancel"
                    onClicked: {
                        flashWorker.cancel();
                        isFlashing = false;
                        flashComplete = false;
                    }
                    visible: isFlashing
                }
                
                Button {
                    id: flashButton
                    text: i18n("Start Flashing")
                    icon.name: "media-flash"
                    highlighted: true
                    enabled: isoPath && deviceCombo.currentIndex > 0 && !isFlashing && !flashComplete
                    
                    onClicked: {
                        isFlashing = true;
                        flashComplete = false;
                        flashWorker.progress = 0;
                        flashWorker.error = "";
                        flashWorker.startFlashing();
                    }
                }
                
                Button {
                    id: doneButton
                    text: i18n("Done")
                    icon.name: "dialog-ok-apply"
                    onClicked: {
                        // Reset and go to home
                        flashComplete = false;
                        pageStack.pop();
                    }
                    visible: flashComplete
                }
            }
        }
    }
    
    // Initialize with data from previous page
    Component.onCompleted: {
        // This would be set when navigating to this page
        // isoPath = pageStack.currentItem.isoPath;
        // devicePath = pageStack.currentItem.devicePath;
    }
}