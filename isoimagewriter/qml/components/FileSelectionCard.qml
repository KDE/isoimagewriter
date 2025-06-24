import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

Rectangle {
    id: root
    height: 120
    radius: 16
    color: Kirigami.Theme.backgroundColor
    border.width: 1
    border.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)
    
    property string title: ""
    property string fieldText: ""
    property alias textField: textFieldContainer
    
    signal browseClicked()
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.gridUnit * 2
        spacing: Kirigami.Units.gridUnit
        
        Text {
            text: root.title
            font.pixelSize: 14
            font.weight: Font.Medium
            color: Kirigami.Theme.textColor
        }
        
        RowLayout {
            Layout.fillWidth: true
            
            Item {
                id: textFieldContainer
                Layout.fillWidth: true
                height: 40
            }
            
            Button {
                text: "Browse"
                highlighted: true
                onClicked: root.browseClicked()
            }
        }
    }
}