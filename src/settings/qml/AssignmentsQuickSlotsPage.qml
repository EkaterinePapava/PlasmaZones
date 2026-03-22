// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Flickable {
    id: root

    readonly property var
    settingsBridge: QtObject {
        readonly property bool autotileEnabled: appSettings.autotileEnabled
        readonly property var layouts: settingsController.layouts
        property int assignmentViewMode: 0

        signal quickLayoutSlotsChanged()
        signal tilingQuickLayoutSlotsChanged()

        // Quick layout slots
        function getQuickLayoutSlot(n) {
            return settingsController.getQuickLayoutSlot(n);
        }

        function setQuickLayoutSlot(n, id) {
            settingsController.setQuickLayoutSlot(n, id);
            quickLayoutSlotsChanged();
        }

        function getQuickLayoutShortcut(n) {
            return settingsController.getQuickLayoutShortcut(n);
        }

        function getTilingQuickLayoutSlot(n) {
            return settingsController.getTilingQuickLayoutSlot(n);
        }

        function setTilingQuickLayoutSlot(n, id) {
            settingsController.setTilingQuickLayoutSlot(n, id);
            tilingQuickLayoutSlotsChanged();
        }

    }

    // View mode: 0 = snapping (zone layouts), 1 = tiling (autotile algorithms)
    readonly property int viewMode: appSettings.autotileEnabled ? root.settingsBridge.assignmentViewMode : 0

    contentHeight: mainCol.implicitHeight
    clip: true

    QtObject {
        id: constants

        readonly property real labelSecondaryOpacity: 0.7
        readonly property int quickLayoutSlotCount: 9
    }

    ColumnLayout {
        id: mainCol

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: i18n("Assign layouts to keyboard shortcuts for quick switching.")
            visible: true
        }

        // Mode selector (visible only when autotiling is enabled)
        Item {
            Layout.fillWidth: true
            implicitHeight: modeSelectorCard.implicitHeight
            visible: root.settingsBridge.autotileEnabled

            SettingsCard {
                id: modeSelectorCard

                anchors.fill: parent
                headerText: i18n("Configuration Mode")

                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.smallSpacing

                    WideComboBox {
                        id: modeCombo

                        Layout.fillWidth: true
                        model: [{
                            "text": i18n("Snapping — Zone layouts"),
                            "value": 0
                        }, {
                            "text": i18n("Tiling — Autotile algorithms"),
                            "value": 1
                        }]
                        textRole: "text"
                        valueRole: "value"
                        currentIndex: root.settingsBridge.assignmentViewMode
                        onActivated: {
                            root.settingsBridge.assignmentViewMode = model[currentIndex].value;
                        }
                        ToolTip.visible: hovered
                        ToolTip.delay: Kirigami.Units.toolTipDelay
                        ToolTip.text: i18n("Switch between snapping and tiling configurations. Both are saved independently.")
                    }

                    Kirigami.InlineMessage {
                        Layout.fillWidth: true
                        Layout.margins: Kirigami.Units.smallSpacing
                        Layout.topMargin: Kirigami.Units.smallSpacing * 2
                        visible: true
                        type: root.viewMode === 1 ? Kirigami.MessageType.Positive : Kirigami.MessageType.Information
                        text: root.viewMode === 1 ? i18n("Tiling mode: assign autotile algorithms to quick slots.") : i18n("Snapping mode: assign zone layouts to quick slots.")
                    }

                }

            }

        }

        // Quick Layout Shortcuts
        Item {
            Layout.fillWidth: true
            implicitHeight: quickSlotsCard.implicitHeight

            QuickLayoutSlotsCard {
                id: quickSlotsCard

                anchors.fill: parent
                appSettings: root.settingsBridge
                constants: constants
                viewMode: root.viewMode
            }

        }

    }

}
