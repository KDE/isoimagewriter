/*
* SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
* SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 
import QtQuick.Controls as Controls
import QtQuick.Layouts 
import org.kde.isoimagewriter 1.0
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardPage {
    id: root

    property var releases: []
    property bool isLoading: true

    function selectRelease(release) {
        applicationWindow().pageStack.push("qrc:/qml/pages/DownloadWriteOptionsPage.qml", {
            "isoName": release.name,
            "isoUrl": release.url,
            "isoHash": release.sha256,
            "isoHashAlgo": "sha256"
        });
    }

    function groupReleasesByDistro() {
        let distroGroups = {
        };
        if (!releases || !releases.length)
            return distroGroups;

        for (let i = 0; i < releases.length; i++) {
            let release = releases[i];
            if (!distroGroups[release.distro])
                distroGroups[release.distro] = [];

            distroGroups[release.distro].push(release);
        }
        return distroGroups;
    }

    title: i18nc("@title:window", "Browse Linux Distributions")
    Component.onCompleted: {
        console.log("Starting release fetch…");
        releaseFetcher.fetchReleases();
    }

    ReleaseFetch {
        id: releaseFetcher

        onReleasesReady: function(fetchedReleases) {
            console.log("Releases fetched:", fetchedReleases.length);
            root.releases = fetchedReleases;
            root.isLoading = false;
        }
        onFetchFailed: function(error) {
            console.log("Failed to fetch releases:", error);
            root.isLoading = false;
            showPassiveNotification(i18nc("@info:status", "Failed to fetch releases: %1", error));
        }
        onFetchProgress: function(status) {
            console.log("Fetch progress:", status);
        }
    }

    // Main content layout, visible only when not loading
    ColumnLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        spacing: 0
        visible: !isLoading

        // Show releases if available
        ColumnLayout {
            visible: releases && releases.length > 0
            spacing: 0
            Layout.fillWidth: true

            FormCard.FormHeader {
                title: "Select ISO"
                Layout.topMargin: Kirigami.Units.gridUnit
            }

            Repeater {
                model: {
                    if (releases && releases.length > 0) {
                        let distroGroups = groupReleasesByDistro();
                        return Object.keys(distroGroups).map((distroName) => {
                            return ({
                                "distroName": distroName,
                                "releases": distroGroups[distroName]
                            });
                        });
                    }
                    return [];
                }

                delegate: ColumnLayout {
                    required property var modelData

                    spacing: 0

                    FormCard.FormCard {
                        Repeater {
                            model: modelData.releases

                            delegate: ColumnLayout {
                                required property var modelData
                                required property int index

                                spacing: 0

                                FormCard.FormButtonDelegate {
                                    text: modelData.name + (modelData.version ? " (" + modelData.version + ")" : "")
                                    onClicked: root.selectRelease(modelData)
                                }

                            }

                        }

                    }

                }

            }

        }

        // Spacer before download button
        Item {
            height: Kirigami.Units.largeSpacing
            visible: releases && releases.length > 0
        }

        // Always show download from URL option when not loading
        FormCard.FormCard {
            Layout.fillWidth: true

            FormCard.FormButtonDelegate {
                text: "Download from URL"
                icon.name: "globe"
                highlighted: true
                onClicked: urlDialog.open()
            }

        }

        FormCard.FormSectionText {
            Layout.fillWidth: true
            text: i18nc("@info", "For more distributions, visit the <a href=\"https://kde.org/distributions\"><span style=\" text-decoration: underline;\">KDE Distributions</span></a>")
        }

    }

    // Loading state - centered on the page
    Controls.BusyIndicator {
        id: indicator

        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        Layout.fillWidth: true
        Layout.fillHeight: true
        running: isLoading
        visible: isLoading
    }

    // URL Input Dialog
    Kirigami.Dialog {
        id: urlDialog

        title: i18nc("@title:window", "Add URL")
        standardButtons: Kirigami.Dialog.NoButton
        preferredHeight: Kirigami.Units.gridUnit * 10
        preferredWidth: Kirigami.Units.gridUnit * 15
        customFooterActions: [
            Kirigami.Action {
                // nameInput is not defined in the provided code, so it has been removed.

                text: i18nc("@action:button", "Next")
                icon.name: "go-next"
                enabled: urlInput.isValidUrl
                onTriggered: {
                    applicationWindow().pageStack.push("qrc:/qml/pages/DownloadWriteOptionsPage.qml", {
                        "isoName": urlInput.text,
                        "isoUrl": urlInput.text.trim(),
                        "isoHash": "",
                        "isoHashAlgo": ""
                    });
                    urlDialog.close();
                    urlInput.text = "";
                }
            }
        ]

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            Kirigami.InlineMessage {
                Layout.fillWidth: true
                type: Kirigami.MessageType.Information
                text: i18nc("@info", "Enter the direct download URL of the Image File")
                visible: true
            }

            Controls.Label {
                Layout.fillWidth: true
                text: i18nc("@label", "ISO URL:")
                font.bold: true
            }

            Controls.TextField {
                id: urlInput

                // Validate URL format
                property bool isValidUrl: {
                    let urlText = text.trim();
                    return urlText.length > 0 && (urlText.startsWith("http://") || urlText.startsWith("https://")) && urlText.toLowerCase().endsWith(".iso");
                }

                Layout.fillWidth: true
                placeholderText: i18nc("@info:placeholder", "https://example.com/path/to/file.iso")
                selectByMouse: true
            }

        }

    }

}
