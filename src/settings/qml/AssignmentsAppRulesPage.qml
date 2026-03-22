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

        signal appRulesRefreshed()

        // App rules
        function getAppRulesForLayout(id) {
            return settingsController.getAppRulesForLayout(id);
        }

        function addAppRuleToLayout(id, pattern, zone, screen) {
            settingsController.addAppRuleToLayout(id, pattern, zone, screen);
            appRulesRefreshed();
        }

        function removeAppRuleFromLayout(id, idx) {
            settingsController.removeAppRuleFromLayout(id, idx);
            appRulesRefreshed();
        }

        // Running windows
        function getRunningWindows() {
            return settingsController.getRunningWindows();
        }

    }

    // App rules are snapping-only (zone-based), so viewMode is always 0
    readonly property int viewMode: 0

    contentHeight: mainCol.implicitHeight
    clip: true

    QtObject {
        id: constants

        readonly property real labelSecondaryOpacity: 0.7
    }

    WindowPickerDialog {
        id: windowPickerDialog

        appSettings: root.settingsBridge
    }

    ColumnLayout {
        id: mainCol

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: i18n("Define rules to auto-snap windows to specific zones when they open.")
            visible: true
        }

        // App-to-zone rules
        Item {
            Layout.fillWidth: true
            implicitHeight: appRulesCard.implicitHeight

            AppRulesCard {
                id: appRulesCard

                anchors.fill: parent
                appSettings: root.settingsBridge
                constants: constants
                windowPickerDialog: windowPickerDialog
                viewMode: root.viewMode
            }

        }

    }

}
