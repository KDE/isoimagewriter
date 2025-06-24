import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 70
    radius: 12
    color: isActive ? Qt.rgba(Kirigami.Theme.positiveTextColor.r, Kirigami.Theme.positiveTextColor.g, Kirigami.Theme.positiveTextColor.b, 0.1) : Kirigami.Theme.backgroundColor
    border.width: 1
    border.color: isActive ? Kirigami.Theme.positiveTextColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)
    
    property string title: ""
    property string icon: ""
    property bool isActive: false
    
    signal clicked()
    
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        
        onEntered: parent.color = Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
        onExited: parent.color = isActive ? Qt.rgba(Kirigami.Theme.positiveTextColor.r, Kirigami.Theme.positiveTextColor.g, Kirigami.Theme.positiveTextColor.b, 0.1) : Kirigami.Theme.backgroundColor
        
        onClicked: root.clicked()
    }
    
    RowLayout {
        anchors.centerIn: parent
        spacing: Kirigami.Units.gridUnit
        
        Kirigami.Icon {
            width: 24
            height: 24
            source: root.icon
            color: Kirigami.Theme.textColor
        }
        
        Text {
            text: root.title
            font.pixelSize: 16
            color: Kirigami.Theme.textColor
        }
    }
}