/*
* SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
* SPDX-License-Identifier: GPL-3.0-or-later
*/

import QtQuick 
import QtQuick.Controls as Controls
import QtQuick.Layouts 
import org.kde.isoimagewriter
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

    Item {
        Layout.fillHeight: true
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Select ISO")
    }

    Repeater {
        model: {
            if (!root.isLoading && releases && releases.length > 0) {
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

        delegate: FormCard.FormCard {
            id: distroDelegate

            required property int index
            required property var modelData

            Layout.topMargin: index !== 0 ? Kirigami.Units.largeSpacing : 0

            Repeater {
                model: distroDelegate.modelData.releases

                delegate: FormCard.FormButtonDelegate {
                    required property var modelData
                    required property int index

                    text: modelData.name + (modelData.version ? " (" + modelData.version + ")" : "")
                    onClicked: root.selectRelease(modelData)
                }
            }
        }
    }

    FormCard.FormCard {
        visible: root.isLoading
        FormCard.FormPlaceholderMessageDelegate {
            id: indicator
            text: i18nc("@info:placeholder", "Loading list of distros")
        }
    }

    FormCard.FormCard {
        Layout.fillWidth: true
        Layout.topMargin: Kirigami.Units.largeSpacing

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

    FormCard.FormCardDialog {
        id: urlDialog

        title: i18nc("@title:dialog", "Add URL")
        standardButtons: Controls.Dialog.Ok | Controls.Dialog.Cancel

        onAccepted: {
            root.Kirigami.PageStack.push(Qt.createComponent("org.kde.isoimagewriter", "DownloadWriteOptionsPage"), {
                "isoName": urlInput.text,
                "isoUrl": urlInput.text.trim(),
                "isoHash": "",
                "isoHashAlgo": ""
            });
            urlDialog.close();
            urlInput.text = '';
        }

        onRejected: {
            urlDialog.close();
            urlInput.text = '';
        }

        Component.onCompleted: {
            const nextButton = standardButton(Kirigami.Dialog.Ok);
            nextButton.text = i18nc("@action:button", "Next");
            nextButton.icon.name = "go-next-symbolic";
            nextButton.enabled = Qt.binding(() => urlInput.isValidUrl)
        }

        FormCard.FormSectionText {
            text: i18nc("@info", "Enter the direct download URL of the Image File")
        }

        FormCard.FormTextFieldDelegate {
            id: urlInput

            label: i18nc("@label", "ISO URL:")
            placeholderText: i18nc("@info:placeholder", "https://example.com/path/to/file.iso")

            // Validate URL format
            property bool isValidUrl: {
                let urlText = text.trim();
                return urlText.length > 0 && (urlText.startsWith("http://") || urlText.startsWith("https://")) && urlText.toLowerCase().endsWith(".iso");
            }
        }
    }

    Item {
        Layout.fillHeight: true
    }
}
