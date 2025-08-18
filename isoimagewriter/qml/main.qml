/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs
import QtQuick.Layouts

import "pages"

Kirigami.ApplicationWindow {
    id: root
    title: i18n("ISO Image Writer")
    minimumWidth: Kirigami.Units.gridUnit * 24
    minimumHeight: Kirigami.Units.gridUnit * 12
    width: Kirigami.Units.gridUnit * 28
    height: Kirigami.Units.gridUnit * 18

    pageStack.globalToolBar.showNavigationButtons: 1
    pageStack.defaultColumnWidth: root.width
    pageStack.initialPage: WelcomePage {}
}
