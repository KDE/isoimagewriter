import QtQuick 2.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.19 as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts 1.15

import "pages"

Kirigami.ApplicationWindow {
    id: root
    title: "ISO Image Writer"
    width: 800
    height: 600
    minimumWidth: 700
    minimumHeight: 500

    pageStack.defaultColumnWidth: root.width
    pageStack.initialPage: WelcomePage {}
}
