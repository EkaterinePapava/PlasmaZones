// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Flickable {
    id: root

    readonly property bool bordersActive: hideTitleBarsCheck.checked || showBorderCheck.checked

    contentHeight: content.implicitHeight
    clip: true

    ColumnLayout {
        id: content

        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // =================================================================
        // Colors Card
        // =================================================================
        SettingsCard {
            Layout.fillWidth: true
            headerText: i18n("Colors")
            collapsible: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                SettingsRow {
                    title: i18n("Use system accent color")
                    description: i18n("Derive border colors from your system color scheme")

                    SettingsSwitch {
                        id: useSystemColorsCheck

                        checked: appSettings.autotileUseSystemBorderColors
                        accessibleName: i18n("Use system accent color")
                        onToggled: appSettings.autotileUseSystemBorderColors = checked
                    }

                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    visible: !useSystemColorsCheck.checked
                }

                SettingsRow {
                    visible: !useSystemColorsCheck.checked
                    title: i18n("Active border color")
                    description: i18n("Border color for the focused window")

                    ColorButton {
                        color: appSettings.autotileBorderColor
                        onClicked: {
                            activeBorderColorDialog.selectedColor = appSettings.autotileBorderColor;
                            activeBorderColorDialog.open();
                        }
                    }

                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                    visible: !useSystemColorsCheck.checked
                }

                SettingsRow {
                    visible: !useSystemColorsCheck.checked
                    title: i18n("Inactive border color")
                    description: i18n("Border color for unfocused windows")

                    ColorButton {
                        color: appSettings.autotileInactiveBorderColor
                        onClicked: {
                            inactiveBorderColorDialog.selectedColor = appSettings.autotileInactiveBorderColor;
                            inactiveBorderColorDialog.open();
                        }
                    }

                }

            }

        }

        // =================================================================
        // Decorations Card
        // =================================================================
        SettingsCard {
            Layout.fillWidth: true
            headerText: i18n("Decorations")
            collapsible: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                SettingsRow {
                    title: i18n("Hide title bars")
                    description: i18n("Remove window title bars while autotiled, restored when floating")

                    SettingsSwitch {
                        id: hideTitleBarsCheck

                        checked: appSettings.autotileHideTitleBars
                        accessibleName: i18n("Hide title bars on tiled windows")
                        onToggled: appSettings.autotileHideTitleBars = checked
                    }

                }

            }

        }

        // =================================================================
        // Borders Card
        // =================================================================
        SettingsCard {
            Layout.fillWidth: true
            headerText: i18n("Borders")
            showToggle: true
            toggleChecked: appSettings.autotileShowBorder
            onToggleChanged: appSettings.autotileShowBorder = toggleChecked
            collapsible: true

            contentItem: ColumnLayout {
                spacing: Kirigami.Units.smallSpacing

                SettingsRow {
                    title: i18n("Border width")
                    description: i18n("Thickness of colored borders around tiled windows")

                    SettingsSpinBox {
                        from: settingsController.autotileBorderWidthMin
                        to: settingsController.autotileBorderWidthMax
                        value: appSettings.autotileBorderWidth
                        onValueModified: (value) => {
                            return appSettings.autotileBorderWidth = value;
                        }
                    }

                }

                Kirigami.Separator {
                    Layout.fillWidth: true
                    Layout.leftMargin: Kirigami.Units.largeSpacing
                    Layout.rightMargin: Kirigami.Units.largeSpacing
                }

                SettingsRow {
                    title: i18n("Corner radius")
                    description: i18n("Roundness of border corners (0 for square)")

                    SettingsSpinBox {
                        from: settingsController.autotileBorderRadiusMin
                        to: settingsController.autotileBorderRadiusMax
                        value: appSettings.autotileBorderRadius
                        onValueModified: (value) => {
                            return appSettings.autotileBorderRadius = value;
                        }
                    }

                }

            }

        }

    }

    // =====================================================================
    // Color Dialogs
    // =====================================================================
    ColorDialog {
        id: activeBorderColorDialog

        title: i18n("Choose Active Border Color")
        onAccepted: appSettings.autotileBorderColor = selectedColor
    }

    ColorDialog {
        id: inactiveBorderColorDialog

        title: i18n("Choose Inactive Border Color")
        onAccepted: appSettings.autotileInactiveBorderColor = selectedColor
    }

}
