import QtQuick
import QtQuick.Controls 
import QtQuick.Layouts 
import QtQuick.Dialogs 
import org.kde.kirigami as Kirigami
import org.kde.isoimagewriter 1.0
import QtCore

Kirigami.ScrollablePage {
    id: filePage
    title: i18n("Write ISO Image")
    property string preselectedFile: ""
    
    function getVerificationInstructions(filePath) {
        if (!filePath) return ""
        
        const fileName = filePath.toString().split('/').pop()
        
        if (fileName.startsWith("neon-")) {
            return i18n("Neon ISO detected. The signature will be automatically verified using the imported Neon signing key.")
        } else if (fileName.startsWith("archlinux-")) {
            return i18n("Arch Linux ISO detected. The signature will be automatically verified using the imported Arch Linux signing key.")
        } else if (fileName.startsWith("kubuntu-") || fileName.startsWith("ubuntu-")) {
            return i18n("Ubuntu/Kubuntu ISO detected. Please ensure the SHA256SUMS and SHA256SUMS.gpg files are in the same directory as the ISO for verification.")
        } else if (fileName.startsWith("netrunner-") || fileName.startsWith("debian-")) {
            return i18n("Netrunner/Debian ISO detected. You will be prompted to enter the SHA256 checksum for verification.")
        } else if (fileName.endsWith(".iso")) {
            return i18n("Unrecognized ISO. Automatic verification not available. Please verify the checksum manually.")
        }
        return ""
    }

    IsoVerifier {
        id: isoVerifier
        
        onFinished: function(result, error) {
            if (result === IsoVerifier.Successful) {
                verificationCard.state = "success"
                createButton.enabled = true
            } else {
                verificationCard.state = "error"
                verificationCard.errorMessage = error
                createButton.enabled = false
            }
        }
        
        onInputRequested: function(title, body) {
            verificationCard.inputTitle = title
            verificationCard.inputBody = body
            verificationCard.state = "input"
        }
    }
    
    FileDialog {
        id: fileDialog
        title: i18n("Select ISO image")
        nameFilters: ["ISO files (*.iso)", "All files (*)"]
        onAccepted: {
            if (fileDialog.selectedFile) {
                let filePath = fileDialog.selectedFile.toString()
                if (filePath.startsWith("file://")) {
                    //removes the file:// prefix for display
                    filePath = filePath.substring(7)
                }
                
                selectedFileCard.filePath = filePath
                selectedFileCard.fileName = filePath.split('/').pop()
                selectedFileCard.visible = true
                
                isoVerifier.filePath = filePath
                verificationCard.instructions = getVerificationInstructions(filePath)
                verificationCard.visible = true
                verificationCard.state = "ready"
                
                createButton.enabled = false

                skipVerification.enabled = true
            }
        }
    }
    
    Component.onCompleted: {
        if (preselectedFile) {
            selectedFileCard.filePath = preselectedFile
            selectedFileCard.fileName = preselectedFile.split('/').pop()
            selectedFileCard.visible = true
            
            isoVerifier.filePath = preselectedFile
            verificationCard.instructions = getVerificationInstructions(preselectedFile)
            verificationCard.visible = true
            verificationCard.state = "ready"
            
            createButton.enabled = false
        }
    }
    
    ColumnLayout {
        anchors {
            fill: parent
            margins: Kirigami.Units.largeSpacing
        }
        spacing: Kirigami.Units.largeSpacing
        
        RowLayout {
            Kirigami.FormData.label: i18n("Write this ISO image:")
            spacing: Kirigami.Units.smallSpacing
            
            TextField {
                id: isoField
                Layout.fillWidth: true
                placeholderText: i18n("Path to ISO image...")
                readOnly: false
            }
            
            Button {
                icon.name: "folder-open"
                text: i18n("Browse")
                onClicked: fileDialog.open()
            }
        }
        
        Kirigami.Card {
            id: selectedFileCard
            Layout.fillWidth: true
            visible: false
            
            property string filePath: ""
            property string fileName: ""
            
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                
                Kirigami.Heading {
                    level: 3
                    text: i18n("Selected File")
                }
                
                RowLayout {
                    spacing: Kirigami.Units.smallSpacing
                    
                    Kirigami.Icon {
                        source: "application-x-cd-image"
                        width: Kirigami.Units.iconSizes.medium
                        height: Kirigami.Units.iconSizes.medium
                    }
                    
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2
                        
                        Label {
                            text: selectedFileCard.fileName
                            font.weight: Font.Bold
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                        
                        Label {
                            text: selectedFileCard.filePath
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideMiddle
                            Layout.fillWidth: true
                        }
                    }
                    
                    Button {
                        icon.name: "edit-select"
                        text: i18n("Change")
                        onClicked: fileDialog.open()
                    }
                }
            }
        }
        
        Kirigami.Card {
            id: verificationCard
            Layout.fillWidth: true
            visible: false
            
            property string instructions: ""
            property string errorMessage: ""
            property string inputTitle: ""
            property string inputBody: ""
            
            states: [
                State {
                    name: "ready"
                    PropertyChanges {
                        target: verifyButton
                        text: i18n("Verify ISO")
                        enabled: true
                        icon.name: "security-medium"
                    }
                    PropertyChanges {
                        target: statusRow
                        visible: false
                    }
                    PropertyChanges {
                        target: inputSection
                        visible: false
                    }
                },
                State {
                    name: "verifying"
                    PropertyChanges {
                        target: verifyButton
                        text: i18n("Verifying...")
                        enabled: false
                        icon.name: "view-refresh"
                    }
                    PropertyChanges {
                        target: statusRow
                        visible: true
                    }
                    PropertyChanges {
                        target: statusIcon
                        source: "view-refresh"
                    }
                    PropertyChanges {
                        target: statusLabel
                        text: i18n("Verifying ISO integrity...")
                        color: Kirigami.Theme.textColor
                    }
                    PropertyChanges {
                        target: inputSection
                        visible: false
                    }
                },
                State {
                    name: "input"
                    PropertyChanges {
                        target: verifyButton
                        text: i18n("Verify ISO")
                        enabled: true
                        icon.name: "security-medium"
                    }
                    PropertyChanges {
                        target: statusRow
                        visible: false
                    }
                    PropertyChanges {
                        target: inputSection
                        visible: true
                    }
                },
                State {
                    name: "success"
                    PropertyChanges {
                        target: verifyButton
                        text: i18n("Verified")
                        enabled: false
                        icon.name: "emblem-success"
                    }
                    PropertyChanges {
                        target: statusRow
                        visible: true
                    }
                    PropertyChanges {
                        target: statusIcon
                        source: "emblem-success"
                    }
                    PropertyChanges {
                        target: statusLabel
                        text: i18n("ISO verification successful")
                        color: Kirigami.Theme.positiveTextColor
                    }
                    PropertyChanges {
                        target: inputSection
                        visible: false
                    }
                },
                State {
                    name: "error"
                    PropertyChanges {
                        target: verifyButton
                        text: i18n("Retry Verification")
                        enabled: true
                        icon.name: "view-refresh"
                    }
                    PropertyChanges {
                        target: statusRow
                        visible: true
                    }
                    PropertyChanges {
                        target: statusIcon
                        source: "emblem-error"
                    }
                    PropertyChanges {
                        target: statusLabel
                        text: i18n("Verification failed: %1", verificationCard.errorMessage)
                        color: Kirigami.Theme.negativeTextColor
                    }
                    PropertyChanges {
                        target: inputSection
                        visible: false
                    }
                }
            ]
            
            contentItem: ColumnLayout {
                spacing: Kirigami.Units.largeSpacing
                
                Kirigami.Heading {
                    level: 3
                    text: i18n("ISO Verification")
                }
                
                Label {
                    Layout.fillWidth: true
                    text: verificationCard.instructions
                    wrapMode: Text.WordWrap
                    visible: text.length > 0
                }
                
                ColumnLayout {
                    id: inputSection
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    visible: false
                    
                    Label {
                        Layout.fillWidth: true
                        text: verificationCard.inputTitle
                        font.weight: Font.Bold
                        wrapMode: Text.WordWrap
                        visible: text.length > 0
                    }
                    
                    Label {
                        Layout.fillWidth: true
                        text: verificationCard.inputBody
                        wrapMode: Text.WordWrap
                        visible: text.length > 0
                    }
                    
                    TextField {
                        id: checksumField
                        Layout.fillWidth: true
                        placeholderText: i18n("Paste SHA256 checksum here...")
                        
                        onAccepted: {
                            if (text.length > 0) {
                                verificationCard.state = "verifying"
                                isoVerifier.verifyWithInputText(true, text)
                            }
                        }
                    }
                    
                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing
                        
                        Button {
                            text: i18n("Verify with Checksum")
                            icon.name: "security-medium"
                            enabled: checksumField.text.length > 0
                            
                            onClicked: {
                                verificationCard.state = "verifying"
                                isoVerifier.verifyWithInputText(true, checksumField.text)
                            }
                        }
                        
                        Button {
                            text: i18n("Cancel")
                            icon.name: "dialog-cancel"
                            
                            onClicked: {
                                checksumField.text = ""
                                verificationCard.state = "ready"
                                isoVerifier.verifyWithInputText(false, "")
                            }
                        }
                    }
                }
                
                ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing
                    visible: verificationCard.state !== "input"
                    
                    Button {
                        id: verifyButton
                        
                        onClicked: {
                            verificationCard.state = "verifying"
                            isoVerifier.verifyIso()
                        }
                    }
                    
                    RowLayout {
                        id: statusRow
                        spacing: Kirigami.Units.smallSpacing
                        visible: false
                        
                        Kirigami.Icon {
                            id: statusIcon
                            width: Kirigami.Units.iconSizes.small
                            height: Kirigami.Units.iconSizes.small
                        }
                        
                        Label {
                            id: statusLabel
                            Layout.fillWidth: true
                        }
                    }
                }
                
                Label {
                    Layout.fillWidth: true
                    text: i18n("You can skip verification and proceed directly, but it's recommended to verify the ISO integrity first.")
                    wrapMode: Text.WordWrap
                    color: Kirigami.Theme.disabledTextColor
                    visible: verificationCard.state === "ready"
                }
            }
        }
        
    }
    
    footer: ToolBar { 
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }
            
            Button {
                id: skipVerification
                text: i18n("Skip Verification")
                icon.name: "go-next"
                enabled: false
                
                onClicked: {
                    pageStack.push("qrc:/qml/pages/FlashPage.qml", {
                        isoPath: selectedFileCard.filePath
                    });
                }
            }
            
            Button {
                id: createButton
                text: i18n("Next")
                icon.name: "go-next"
                highlighted: true
                enabled: false
                
                onClicked: {
                    pageStack.push("qrc:/qml/pages/FlashPage.qml", {
                        isoPath: selectedFileCard.filePath
                    });
                }
            }
        }
    }
}