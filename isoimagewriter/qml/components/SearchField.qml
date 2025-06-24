import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 60
    radius: 16
    color: Kirigami.Theme.backgroundColor
    border.width: 1
    border.color: textField.activeFocus ? Kirigami.Theme.highlightColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)
    
    property alias searchText: textField.text
    property string placeholderText: "Search for an operating system..."
    
    Behavior on border.color { ColorAnimation { duration: 200 } }
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit
        spacing: Kirigami.Units.gridUnit
        
        Kirigami.Icon {
            width: 20
            height: 20
            source: "search"
            color: Kirigami.Theme.disabledTextColor
        }
        
        TextField {
            id: textField
            Layout.fillWidth: true
            placeholderText: root.placeholderText
            font.pixelSize: 16
            
            background: Rectangle {
                color: "transparent"
                border.width: 0
            }
        }
    }
}