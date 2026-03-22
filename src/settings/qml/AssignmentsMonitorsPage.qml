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
        readonly property string autotileAlgorithm: appSettings.autotileAlgorithm
        readonly property string defaultLayoutId: appSettings.defaultLayoutId
        readonly property var screens: settingsController.screens
        readonly property var layouts: settingsController.layouts
        property int assignmentViewMode: 0
        readonly property int virtualDesktopCount: settingsController.virtualDesktopCount
        readonly property var virtualDesktopNames: settingsController.virtualDesktopNames
        readonly property var disabledMonitors: appSettings.disabledMonitors

        signal screenAssignmentsChanged()
        signal tilingScreenAssignmentsChanged()
        signal tilingDesktopAssignmentsChanged()
        signal lockedScreensChanged()

        function isMonitorDisabled(name) {
            return settingsController.isMonitorDisabled(name);
        }

        function setMonitorDisabled(name, disabled) {
            settingsController.setMonitorDisabled(name, disabled);
        }

        function assignLayoutToScreen(screen, layout) {
            settingsController.assignLayoutToScreen(screen, layout);
            screenAssignmentsChanged();
        }

        function clearScreenAssignment(screen) {
            settingsController.clearScreenAssignment(screen);
            screenAssignmentsChanged();
        }

        function assignTilingLayoutToScreen(screen, layout) {
            settingsController.assignTilingLayoutToScreen(screen, layout);
            tilingScreenAssignmentsChanged();
        }

        function clearTilingScreenAssignment(screen) {
            settingsController.clearTilingScreenAssignment(screen);
            tilingScreenAssignmentsChanged();
        }

        function getLayoutForScreen(screen) {
            return settingsController.getLayoutForScreen(screen);
        }

        function getTilingLayoutForScreen(screen) {
            return settingsController.getTilingLayoutForScreen(screen);
        }

        function isScreenLocked(screen, mode) {
            return settingsController.isScreenLocked(screen, mode);
        }

        function toggleScreenLock(screen, mode) {
            settingsController.toggleScreenLock(screen, mode);
            lockedScreensChanged();
        }

        function isContextLocked(screen, desktop, activity, mode) {
            return settingsController.isContextLocked(screen, desktop, activity, mode);
        }

        function toggleContextLock(screen, desktop, activity, mode) {
            settingsController.toggleContextLock(screen, desktop, activity, mode);
            lockedScreensChanged();
        }

        // Per-desktop assignments
        function hasExplicitAssignmentForScreenDesktop(screen, desktop) {
            return settingsController.hasExplicitAssignmentForScreenDesktop(screen, desktop);
        }

        function hasExplicitTilingAssignmentForScreenDesktop(screen, desktop) {
            return settingsController.hasExplicitTilingAssignmentForScreenDesktop(screen, desktop);
        }

        function getLayoutForScreenDesktop(screen, desktop) {
            return settingsController.getLayoutForScreenDesktop(screen, desktop);
        }

        function getSnappingLayoutForScreenDesktop(screen, desktop) {
            return settingsController.getSnappingLayoutForScreenDesktop(screen, desktop);
        }

        function getTilingLayoutForScreenDesktop(screen, desktop) {
            return settingsController.getTilingLayoutForScreenDesktop(screen, desktop);
        }

        function clearScreenDesktopAssignment(screen, desktop) {
            settingsController.clearScreenDesktopAssignment(screen, desktop);
            screenAssignmentsChanged();
        }

        function assignLayoutToScreenDesktop(screen, desktop, layout) {
            settingsController.assignLayoutToScreenDesktop(screen, desktop, layout);
            screenAssignmentsChanged();
        }

        function assignTilingLayoutToScreenDesktop(screen, desktop, layout) {
            settingsController.assignTilingLayoutToScreenDesktop(screen, desktop, layout);
            tilingScreenAssignmentsChanged();
        }

        function clearTilingScreenDesktopAssignment(screen, desktop) {
            settingsController.clearTilingScreenDesktopAssignment(screen, desktop);
            tilingScreenAssignmentsChanged();
        }

    }

    // View mode: 0 = snapping (zone layouts), 1 = tiling (autotile algorithms)
    readonly property int viewMode: appSettings.autotileEnabled ? root.settingsBridge.assignmentViewMode : 0

    contentHeight: mainCol.implicitHeight
    clip: true

    QtObject {
        id: constants

        readonly property real labelSecondaryOpacity: 0.7
    }

    ColumnLayout {
        id: mainCol

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: i18n("Assign layouts to monitors and virtual desktops.")
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
                        text: root.viewMode === 1 ? i18n("Tiling mode: assign autotile algorithms to each monitor. These are used when tiling is active.") : i18n("Snapping mode: assign zone layouts to each monitor. These are used when dragging windows.")
                    }

                }

            }

        }

        // Monitor Assignments
        Item {
            Layout.fillWidth: true
            implicitHeight: monitorCard.implicitHeight

            MonitorAssignmentsCard {
                id: monitorCard

                anchors.fill: parent
                appSettings: root.settingsBridge
                constants: constants
                viewMode: root.viewMode
            }

        }

    }

}
