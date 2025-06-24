import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 80
    radius: 12
    color: Kirigami.Theme.backgroundColor
    border.width: 1
    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)
    
    property var distributionData: null
    
    signal clicked()
    
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onEntered: parent.color = Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
        onExited: parent.color = Kirigami.Theme.backgroundColor
        
        onClicked: root.clicked()
    }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit * 1.5
        spacing: Kirigami.Units.gridUnit
        
        Rectangle {
            width: 40
            height: 40
            radius: 20
            color: Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.2)
            
            Text {
                anchors.centerIn: parent
                text: distributionData ? distributionData.name.charAt(0) : ""
                font.pixelSize: 18
                font.weight: Font.Bold
                color: Kirigami.Theme.highlightColor
            }
        }
        
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            
            Text {
                text: distributionData ? distributionData.name + " (x86_64)" : ""
                font.pixelSize: 16
                font.weight: Font.Medium
                color: Kirigami.Theme.textColor
            }
            
            Text {
                text: distributionData ? distributionData.desc : ""
                font.pixelSize: 12
                color: Kirigami.Theme.disabledTextColor
            }
        }
        
        Kirigami.Icon {
            width: 16
            height: 16
            source: "arrow-right"
            color: Kirigami.Theme.disabledTextColor
        }
    }
}