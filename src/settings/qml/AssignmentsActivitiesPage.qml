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
        readonly property var screens: settingsController.screens
        readonly property var layouts: settingsController.layouts
        property int assignmentViewMode: 0
        readonly property bool activitiesAvailable: settingsController.activitiesAvailable
        readonly property var activities: settingsController.activities
        readonly property string currentActivity: settingsController.currentActivity

        signal activityAssignmentsChanged()
        signal tilingActivityAssignmentsChanged()
        signal screenAssignmentsChanged()
        signal tilingScreenAssignmentsChanged()
        signal lockedScreensChanged()

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

        // Per-activity assignments
        function hasExplicitAssignmentForScreenActivity(screen, activity) {
            return settingsController.hasExplicitAssignmentForScreenActivity(screen, activity);
        }

        function hasExplicitTilingAssignmentForScreenActivity(screen, activity) {
            return settingsController.hasExplicitTilingAssignmentForScreenActivity(screen, activity);
        }

        function getLayoutForScreenActivity(screen, activity) {
            return settingsController.getLayoutForScreenActivity(screen, activity);
        }

        function getSnappingLayoutForScreenActivity(screen, activity) {
            return settingsController.getSnappingLayoutForScreenActivity(screen, activity);
        }

        function getTilingLayoutForScreenActivity(screen, activity) {
            return settingsController.getTilingLayoutForScreenActivity(screen, activity);
        }

        function clearScreenActivityAssignment(screen, activity) {
            settingsController.clearScreenActivityAssignment(screen, activity);
            activityAssignmentsChanged();
        }

        function assignLayoutToScreenActivity(screen, activity, layout) {
            settingsController.assignLayoutToScreenActivity(screen, activity, layout);
            activityAssignmentsChanged();
        }

        function assignTilingLayoutToScreenActivity(screen, activity, layout) {
            settingsController.assignTilingLayoutToScreenActivity(screen, activity, layout);
            tilingActivityAssignmentsChanged();
        }

        function clearTilingScreenActivityAssignment(screen, activity) {
            settingsController.clearTilingScreenActivityAssignment(screen, activity);
            tilingActivityAssignmentsChanged();
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
            text: i18n("Assign layouts to KDE Activities per monitor.")
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
                        text: root.viewMode === 1 ? i18n("Tiling mode: assign autotile algorithms per activity.") : i18n("Snapping mode: assign zone layouts per activity.")
                    }

                }

            }

        }

        // Activity Assignments
        Item {
            Layout.fillWidth: true
            implicitHeight: activityCard.implicitHeight
            visible: root.settingsBridge.activitiesAvailable

            ActivityAssignmentsCard {
                id: activityCard

                anchors.fill: parent
                appSettings: root.settingsBridge
                constants: constants
                viewMode: root.viewMode
            }

        }

        // Info when Activities not available
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.smallSpacing
            visible: !root.settingsBridge.activitiesAvailable && root.settingsBridge.screens.length > 0
            type: Kirigami.MessageType.Information
            text: i18n("KDE Activities support is not available. Activity-based layout assignments require the KDE Activities service to be running.")
        }

    }

}
