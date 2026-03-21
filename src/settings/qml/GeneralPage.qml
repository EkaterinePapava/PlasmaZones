// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Flickable {
    id: root

    // Layout constants (previously from monolith's QtObject)
    readonly property int sliderPreferredWidth: Kirigami.Units.gridUnit * 16
    readonly property int sliderValueLabelWidth: Kirigami.Units.gridUnit * 3
    // Capture the context property so child components can access it
    readonly property var settingsBridge: appSettings

    contentHeight: content.implicitHeight
    clip: true
    Component.onCompleted: {
        easingPreview.curve = appSettings.animationEasingCurve;
    }

    ColumnLayout {
        id: content

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // ═══════════════════════════════════════════════════════════════════════
        // ANIMATIONS CARD
        // ═══════════════════════════════════════════════════════════════════════
        Item {
            Layout.fillWidth: true
            implicitHeight: animationsCard.implicitHeight

            SettingsCard {
                id: animationsCard

                anchors.fill: parent
                headerText: i18n("Animations")
                collapsible: true

                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.largeSpacing

                    // Enable toggle
                    CheckBox {
                        id: animationsEnabledCheck

                        Layout.fillWidth: true
                        text: i18n("Smooth window geometry transitions")
                        checked: appSettings.animationsEnabled
                        onToggled: appSettings.animationsEnabled = checked
                        ToolTip.visible: hovered
                        ToolTip.text: i18n("Animate windows when snapping to zones or tiling. Applies to both manual snapping and autotiling.")
                    }

                    // Easing curve editor with animated preview
                    EasingPreview {
                        id: easingPreview

                        Layout.fillWidth: true
                        Layout.maximumWidth: 500
                        Layout.alignment: Qt.AlignHCenter
                        curve: appSettings.animationEasingCurve
                        animationDuration: appSettings.animationDuration
                        previewEnabled: animationsEnabledCheck.checked
                        opacity: animationsEnabledCheck.checked ? 1 : 0.4
                        onCurveEdited: function(newCurve) {
                            appSettings.animationEasingCurve = newCurve;
                        }
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }

                    // Easing controls (extracted component)
                    EasingSettings {
                        Layout.fillWidth: true
                        appSettings: root.settingsBridge
                        constants: root
                        animationsEnabled: animationsEnabledCheck.checked
                        easingPreview: easingPreview
                    }

                }

            }

        }

        // ═══════════════════════════════════════════════════════════════════════
        // ON-SCREEN DISPLAY CARD (extracted component)
        // ═══════════════════════════════════════════════════════════════════════
        OsdCard {
            Layout.fillWidth: true
            appSettings: root.settingsBridge
        }

        // ═══════════════════════════════════════════════════════════════════════
        // CONFIGURATION CARD
        // ═══════════════════════════════════════════════════════════════════════
        SettingsCard {
            headerText: i18n("Configuration")

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                Label {
                    Layout.fillWidth: true
                    text: i18n("Export or import your PlasmaZones settings. Import will create a backup of your current configuration.")
                    wrapMode: Text.WordWrap
                    opacity: 0.7
                }

                RowLayout {
                    Layout.fillWidth: true
                    Layout.topMargin: Kirigami.Units.smallSpacing
                    spacing: Kirigami.Units.largeSpacing

                    Button {
                        text: i18n("Export Settings")
                        icon.name: "document-export"
                        onClicked: exportConfigDialog.open()
                    }

                    Button {
                        text: i18n("Import Settings")
                        icon.name: "document-import"
                        onClicked: importConfigDialog.open()
                    }

                }

            }

        }

    }

    FileDialog {
        id: exportConfigDialog

        title: i18n("Export Settings")
        nameFilters: [i18n("PlasmaZones Config (*.conf *.ini)"), i18n("All files (*)")]
        fileMode: FileDialog.SaveFile
        onAccepted: settingsController.exportAllSettings(selectedFile.toString().replace(/^file:\/\/+/, "/"))
    }

    FileDialog {
        id: importConfigDialog

        title: i18n("Import Settings")
        nameFilters: [i18n("PlasmaZones Config (*.conf *.ini *.rc)"), i18n("All files (*)")]
        fileMode: FileDialog.OpenFile
        onAccepted: settingsController.importAllSettings(selectedFile.toString().replace(/^file:\/\/+/, "/"))
    }

}
