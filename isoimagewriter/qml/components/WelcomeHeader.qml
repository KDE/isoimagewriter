import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Layouts 1.15

ColumnLayout {
    id: root
    spacing: Kirigami.Units.gridUnit * 2
    
    property bool isDragActive: false
    property bool hasValidFile: false
    property string selectedFileName: ""
    
    Rectangle {
        Layout.alignment: Qt.AlignHCenter
        width: 120
        height: 120
        radius: 60
        color: isDragActive ? Kirigami.Theme.highlightColor : Qt.rgba(Kirigami.Theme.highlightColor.r, Kirigami.Theme.highlightColor.g, Kirigami.Theme.highlightColor.b, 0.1)
        border.width: isDragActive ? 3 : 0
        border.color: Kirigami.Theme.highlightColor
        
        Behavior on color { ColorAnimation { duration: 250 } }
        Behavior on border.width { NumberAnimation { duration: 250 } }
        
        Kirigami.Icon {
            anchors.centerIn: parent
            width: 64
            height: 64
            source: "media-optical"
            color: isDragActive ? "white" : Kirigami.Theme.highlightColor
            
            Behavior on color { ColorAnimation { duration: 250 } }
        }
    }
    
    Kirigami.Heading {
        Layout.alignment: Qt.AlignHCenter
        text: isDragActive ? "Drop your ISO here" : "ISO Image Writer"
        level: 1
        font.weight: Font.Light
        color: isDragActive ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
        
        Behavior on color { ColorAnimation { duration: 250 } }
    }
    
    Text {
        Layout.alignment: Qt.AlignHCenter
        text: hasValidFile ? selectedFileName : "Create bootable USB drives with ease"
        color: hasValidFile ? Kirigami.Theme.positiveTextColor : Kirigami.Theme.disabledTextColor
        font.pixelSize: 14
        opacity: isDragActive ? 0.7 : 1.0
        
        Behavior on opacity { OpacityAnimator { duration: 250 } }
    }
}