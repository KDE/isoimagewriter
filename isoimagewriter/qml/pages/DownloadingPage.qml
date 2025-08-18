/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami
import QtQuick.Layouts
import org.kde.isoimagewriter 1.0

Kirigami.Page {
    id: downloadingPage
    title: i18n("Downloading ISO")

    property string isoName: ""
    property string isoUrl: ""
    property string isoHash: ""
    property string isoHashAlgo: ""
    property bool downloadComplete: false
    property string downloadedFilePath: ""

    FetchIsoJob {
        id: fetchJob
        
        onDownloadProgressChanged: function(percentage) {
            progressBar.value = percentage
            progressLabel.text = i18n("Downloading: %1%", percentage)
        }
        
        onFinished: function(filePath) {
            downloadComplete = true
            downloadedFilePath = filePath
            progressLabel.text = i18n("Download completed!")
            statusLabel.text = i18n("ISO downloaded to: %1", filePath)
            continueButton.enabled = true
        }
        
        onFailed: {
            progressLabel.text = i18n("Download failed!")
            statusLabel.text = i18n("Failed to download the ISO file. Please try again.")
            retryButton.visible = true
        }
    }

    Component.onCompleted: {
        if (isoUrl) {
            fetchJob.fetch(isoUrl)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing
        spacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            level: 2
            text: i18n("Downloading %1", isoName)
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.fillHeight: true
        }

        ColumnLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.largeSpacing

            Kirigami.Icon {
                source: "download"
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                Layout.alignment: Qt.AlignHCenter
            }

            Label {
                id: progressLabel
                text: i18n("Starting download...")
                Layout.alignment: Qt.AlignHCenter
                font.pointSize: Kirigami.Theme.defaultFont.pointSize * 1.2
            }

            ProgressBar {
                id: progressBar
                Layout.preferredWidth: 400
                Layout.alignment: Qt.AlignHCenter
                from: 0
                to: 100
                value: 0
            }

            Label {
                id: statusLabel
                text: i18n("Preparing to download ISO file...")
                Layout.alignment: Qt.AlignHCenter
                color: Kirigami.Theme.disabledTextColor
                wrapMode: Text.WordWrap
                Layout.maximumWidth: 400
            }
        }

        Item {
            Layout.fillHeight: true
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: Kirigami.Units.smallSpacing

            Button {
                id: retryButton
                text: i18n("Retry Download")
                icon.name: "view-refresh"
                visible: false
                onClicked: {
                    visible = false
                    progressBar.value = 0
                    progressLabel.text = i18n("Starting download...")
                    statusLabel.text = i18n("Preparing to download ISO file...")
                    fetchJob.fetch(isoUrl)
                }
            }

            Button {
                text: i18n("Cancel")
                icon.name: "dialog-cancel"
                onClicked: {
                    applicationWindow().pageStack.pop()
                }
            }

            Button {
                id: continueButton
                text: i18n("Continue to Flash")
                icon.name: "arrow-right"
                enabled: false
                onClicked: {
                    // Navigate to flash page with the downloaded file
                    applicationWindow().pageStack.push("qrc:/qml/pages/FlashPage.qml", {
                        "isoPath": downloadedFilePath
                    })
                }
            }
        }
    }
}