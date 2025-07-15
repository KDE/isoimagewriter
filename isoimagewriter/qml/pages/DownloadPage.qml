import QtQuick
import QtQuick.Controls 
import org.kde.kirigami as Kirigami
import QtQuick.Layouts 

Kirigami.ScrollablePage {
    id: downloadPage
    title: i18n("Download OS")
    
    property var distributions: [
        { name: "Ubuntu" },
        { name: "Kubuntu" },
        { name: "Fedora" },
        { name: "openSUSE" },
        { name: "Debian" },
        { name: "Linux Mint" },
        { name: "Elementary OS" },
        { name: "Pop!_OS" },
        { name: "Manjaro" }
    ]
    
    property var filteredDistributions: distributions.filter(function(dist) {
        return searchField.text.length === 0 || 
               dist.name.toLowerCase().includes(searchField.text.toLowerCase())
    })
    
    property string selectedDistribution: ""
    
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing

            Kirigami.SearchField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: i18n("Search for an operating system…")
                onTextChanged: {
                    console.log("Search query:", text)
                    // Trigger property binding update
                    downloadPage.filteredDistributions = distributions.filter(function(dist) {
                        return text.length === 0 || 
                               dist.name.toLowerCase().includes(text.toLowerCase())
                    })
                }
            }
        }
    }
    
        ColumnLayout {
        anchors.fill: parent
        spacing: Kirigami.Units.largeSpacing
        
        // Selected distribution info
        Kirigami.InlineMessage {
            id: selectionMessage
            Layout.fillWidth: true
            visible: selectedDistribution.length > 0
            type: Kirigami.MessageType.Information
            text: i18n("Selected: %1", selectedDistribution)
        }
        
        // OS list in a scrollable area
        Flickable {
            id: flickable
            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: width
            contentHeight: contentColumn.height
            clip: true
            
            Column {
                id: contentColumn
                width: parent.width
                spacing: Kirigami.Units.smallSpacing
                
                Repeater {
                    model: filteredDistributions
                    
                    Kirigami.SwipeListItem {
                        id: listItem
                        width: contentColumn.width
                        
                        highlighted: selectedDistribution === modelData.name
                        
                        contentItem: RowLayout {
                            spacing: Kirigami.Units.gridUnit
                            
                            Kirigami.Icon {
                                source: "application-x-cd-image"
                                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            }
                            
                            Label {
                                text: modelData.name
                                Layout.fillWidth: true
                            }
                            
                            Button {
                                text: selectedDistribution === modelData.name ? i18n("Selected") : i18n("Select")
                                icon.name: selectedDistribution === modelData.name ? "checkbox" : "list-add"
                                enabled: selectedDistribution !== modelData.name
                                onClicked: {
                                    selectedDistribution = modelData.name
                                    searchField.text = modelData.name
                                }
                            }
                        }
                        
                        onClicked: {
                            selectedDistribution = modelData.name
                            searchField.text = modelData.name
                        }
                    }
                }
                
                // Empty state
                Kirigami.PlaceholderMessage {
                    width: contentColumn.width
                    anchors.horizontalCenter: parent.horizontalCenter
                    visible: filteredDistributions.length === 0
                    text: i18n("No distributions found")
                    explanation: i18n("Try adjusting your search terms")
                    icon.name: "search"
                }
            }
        }
    }
    
    //TODO: to implement the download function
    // Footer with download button
}