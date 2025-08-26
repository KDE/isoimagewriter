/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts
//TODO: fix automatic redirection to selectionpage when an iso has been dropped

Kirigami.Page {
    id: welcomePage
    title: ""

    property bool isDragActive: dropArea.containsDrag
    property bool hasValidFile: false
    property string selectedFile: ""
    readonly property bool networkConnected: false
    
    globalToolBarStyle: Kirigami.ApplicationHeaderStyle.None
    
    DropArea {
        id: dropArea
        anchors.fill: parent
        
        onEntered: function (drag) {
            if (drag.hasUrls) {
                let hasIso = drag.urls.some(url => url.toString().toLowerCase().endsWith('.iso'));
                hasIso ? drag.accept(Qt.CopyAction) : drag.reject();
            }
        }
        
        onDropped: function (drop) {
            let isoFile = drop.urls.find(url => url.toString().toLowerCase().endsWith('.iso'));
            if (isoFile) {
                welcomePage.selectedFile = isoFile.toString().replace("file://", "");
                welcomePage.hasValidFile = true;
            }
        }
    }
    
    // Main content area
    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(welcomePage.width - Kirigami.Units.gridUnit * 4, Kirigami.Units.gridUnit * 35)
        spacing: Kirigami.Units.largeSpacing * 1.5
        
        // Header section with icon and title
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.largeSpacing
            
            Item {
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                
                Kirigami.Icon {
                    anchors.fill: parent
                    source: welcomePage.isDragActive ? "document-import" : "qrc:/qml/images/org.kde.isoimagewriter.svg"
                    
                }
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                spacing: Kirigami.Units.smallSpacing
                
                Kirigami.Heading {
                    Layout.fillWidth: true
                    text: welcomePage.isDragActive ? i18n("Drop your ISO here") : i18n("KDE ISO Image Writer")
                    color: welcomePage.isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    level: 1
                    font.weight: Font.Bold
                    
                    Behavior on color {
                        ColorAnimation {
                            duration: Kirigami.Units.shortDuration
                        }
                    }
                }
                
                Label {
                    Layout.fillWidth: true
                    text: welcomePage.hasValidFile 
                          ? i18n("Ready to write: %1", welcomePage.selectedFile.split('/').pop())
                          : i18n("A quick and simple way to create bootable USB drive")
                    color: welcomePage.hasValidFile 
                           ? Kirigami.Theme.positiveTextColor 
                           : Kirigami.Theme.textColor
                    wrapMode: Text.WordWrap
                    font.pointSize: Kirigami.Theme.defaultFont.pointSize * 0.95
                }
            }
        }
        
        
        // Action buttons section
        ColumnLayout {
            Layout.fillWidth: true
            Layout.topMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing
            
            // Primary action - Download ISOs
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                
                text: i18n("Browse Linux Distributions")
                icon.name: "download"
                font.bold: true
                
                // Make this the primary action button
                highlighted: true
                
                ToolTip.text: i18n("Download popular Linux distributions like Kubuntu, Fedora, and more")
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                
                onClicked: pageStack.push("qrc:/qml/pages/IsoListingPage.qml")
            }
            
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: Kirigami.Units.gridUnit * 3
                
                text: welcomePage.hasValidFile 
                      ? i18n("Use Selected ISO File")
                      : i18n("Choose Local ISO File")
                icon.name: welcomePage.hasValidFile ? "media-optical" : "document-open"
                
                ToolTip.text: i18n("Select an ISO file from your computer")
                ToolTip.visible: hovered
                ToolTip.delay: 1000
                
                onClicked: {
                    if (welcomePage.hasValidFile) {
                        let page = pageStack.push("qrc:/qml/pages/SelectionPage.qml");
                        page.preselectedFile = welcomePage.selectedFile;
                    } else {
                        pageStack.push("qrc:/qml/pages/SelectionPage.qml");
                    }
                }
            }
        }
        
    }
    
    Button {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.rightMargin: Kirigami.Units.mediumSpacing
        
        text: i18nc("@action:button", "About")
        icon.name: "help-about"
        
        onClicked: pageStack.push("qrc:/qml/pages/AboutPage.qml")
    }
    
}
