import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 60
    radius: 12
    color: Qt.rgba(Kirigami.Theme.neutralTextColor.r, Kirigami.Theme.neutralTextColor.g, Kirigami.Theme.neutralTextColor.b, 0.1)
    border.width: 1
    border.color: Qt.rgba(Kirigami.Theme.neutralTextColor.r, Kirigami.Theme.neutralTextColor.g, Kirigami.Theme.neutralTextColor.b, 0.3)
    
    property string message: ""
    property string iconSource: "dialog-warning"
    
    RowLayout {
        anchors.centerIn: parent
        spacing: Kirigami.Units.gridUnit
        
        Kirigami.Icon {
            width: 20
            height: 20
            source: root.iconSource
            color: Kirigami.Theme.neutralTextColor
        }
        
        Text {
            text: root.message
            color: Kirigami.Theme.neutralTextColor
            font.pixelSize: 14
        }
    }
}