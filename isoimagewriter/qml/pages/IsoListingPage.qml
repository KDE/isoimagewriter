/*
 * SPDX-FileCopyrightText: 2025 Akki <asa297@sfu.ca>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as Controls
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Layouts
import org.kde.isoimagewriter 1.0

FormCard.FormCardPage {
    id: root
    title: i18n("Browse Linux Distributions")

    property var releases: []
    property bool isLoading: true

    ReleaseFetch {
        id: releaseFetcher

        onReleasesReady: function (fetchedReleases) {
            console.log("Releases fetched:", fetchedReleases.length);
            root.releases = fetchedReleases;
            root.isLoading = false;
        }

        onFetchFailed: function (error) {
            console.log("Failed to fetch releases:", error);
            root.isLoading = false;
            showPassiveNotification(i18n("Failed to fetch releases: %1", error));
        }

        onFetchProgress: function (status) {
            console.log("Fetch progress:", status);
        }
    }

    function selectRelease(release) {
        applicationWindow().pageStack.push("qrc:/qml/pages/DownloadWriteOptionsPage.qml", {
            "isoName": release.name,
            "isoUrl": release.url,
            "isoHash": release.sha256,
            "isoHashAlgo": "sha256"
        });
    }

    function groupReleasesByDistro() {
        let distroGroups = {};
        if (!releases || !releases.length) {
            return distroGroups;
        }
        for (let i = 0; i < releases.length; i++) {
            let release = releases[i];
            if (!distroGroups[release.distro]) {
                distroGroups[release.distro] = [];
            }
            distroGroups[release.distro].push(release);
        }
        return distroGroups;
    }

    Item {
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: isLoading || (!releases || releases.length === 0)

        Kirigami.LoadingPlaceholder {
            anchors.centerIn: parent
            visible: isLoading
            text: i18n("Fetching latest releases...")
        }

        // check for internet
        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: !isLoading && (releases ? releases.length === 0 : true)
            text: i18n("No releases available")
            icon.name: "download"
        }
    }

    // Main content - only visible when not loading and has releases

    ColumnLayout {
        visible: !isLoading && releases && releases.length > 0
        spacing: 0

        FormCard.FormHeader {
            visible: !isLoading && releases && releases.length > 0
            title: "Select ISO"
            Layout.topMargin: Kirigami.Units.gridUnit
        }

        Repeater {
            model: {
                if (!isLoading && releases && releases.length > 0) {
                    let distroGroups = groupReleasesByDistro();
                    return Object.keys(distroGroups).map(distroName => ({
                                distroName: distroName,
                                releases: distroGroups[distroName]
                            }));
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

                            FormCard.FormDelegateSeparator {
                                visible: index < parent.parent.model.length - 1
                            }
                        }
                    }
                }
            }
        }
        FormCard.FormSectionText {
            text: i18n("For more distributions, visit the <a href=\"https://kde.org/distributions\"><span style=\" text-decoration: underline;\">KDE Distributions</span></a>")
        }
    }

    Component.onCompleted: {
        console.log("Starting release fetch …");
        releaseFetcher.fetchReleases();
    }
}
