// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Flickable {
    id: root

    // Zone selector constants (consumed by ZoneSelectorSection via constants property)
    readonly property int sliderValueLabelWidth: Kirigami.Units.gridUnit * 3
    readonly property int zoneSelectorTriggerMax: 200
    readonly property int zoneSelectorPreviewWidthMin: 80
    readonly property int zoneSelectorPreviewWidthMax: 400
    readonly property int zoneSelectorPreviewHeightMin: 60
    readonly property int zoneSelectorPreviewHeightMax: 300
    readonly property int zoneSelectorGridColumnsMax: 10
    readonly property real screenAspectRatio: Screen.width > 0 && Screen.height > 0 ? (Screen.width / Screen.height) : (16 / 9)

    contentHeight: content.implicitHeight
    clip: true

    ColumnLayout {
        id: content

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // =====================================================================
        // ZONE SELECTOR (per-monitor popup configuration)
        // =====================================================================
        ZoneSelectorSection {
            Layout.fillWidth: true
            appSettings: settingsController.settings
            controller: settingsController
            constants: root
            isCurrentTab: true
            screenAspectRatio: root.screenAspectRatio
        }

    }

}
