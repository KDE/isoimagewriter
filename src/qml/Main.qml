/*
* SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
* SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.isoimagewriter
import org.kde.kirigami as Kirigami
import "pages"

Kirigami.ApplicationWindow {
    id: root

    title: i18nc("@title:window", "ISO Image Writer")
    minimumWidth: Kirigami.Units.gridUnit * 20
    minimumHeight: Kirigami.Units.gridUnit * 18
    width: Kirigami.Units.gridUnit * 33
    height: Kirigami.Units.gridUnit * 20

    pageStack {
        defaultColumnWidth: root.width

        initialPage: WelcomePage {}

        globalToolBar {
            style: Kirigami.ApplicationHeaderStyle.ToolBar
            showNavigationButtons: if (root.pageStack.currentIndex > 0) {
                return Kirigami.ApplicationHeaderStyle.ShowBackButton;
            } else {
                return 0;
            }
        }
    }
}
