import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts

import "pages"

Kirigami.ApplicationWindow {
    id: root
    title: "ISO Image Writer"
    width: 500
    height: 300
    // minimumWidth: 700
    // minimumHeight: 500
    
    pageStack.globalToolBar.style: Kirigami.ApplicationHeaderStyle.None
    
    pageStack.defaultColumnWidth: root.width
    pageStack.initialPage: WelcomePage {}
}


