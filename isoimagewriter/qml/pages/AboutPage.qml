import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.isoimagewriter.about 1.0
import QtQuick.Controls
import "../components"

Kirigami.AboutPage {
    aboutData: About
    
    Button {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        text: "Back"
        onClicked: pageStack.pop()
    }
}
