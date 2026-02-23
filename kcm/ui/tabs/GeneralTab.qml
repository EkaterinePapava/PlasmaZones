// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ".."

/**
 * @brief General tab - Mode-agnostic settings (OSD)
 *
 * Contains settings that apply to both snapping and tiling modes,
 * such as on-screen display configuration.
 */
ScrollView {
    id: root

    required property var kcm
    required property QtObject constants

    clip: true
    contentWidth: availableWidth

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // On-Screen Display card
        Item {
            Layout.fillWidth: true
            implicitHeight: osdCard.implicitHeight

            Kirigami.Card {
                id: osdCard
                anchors.fill: parent

                header: Kirigami.Heading {
                    level: 3
                    text: i18n("On-Screen Display")
                    padding: Kirigami.Units.smallSpacing
                }

                contentItem: Kirigami.FormLayout {
                    CheckBox {
                        id: showOsdCheckbox
                        Kirigami.FormData.label: i18n("Layout switch:")
                        text: i18n("Show OSD when switching layouts")
                        checked: kcm.showOsdOnLayoutSwitch
                        onToggled: kcm.showOsdOnLayoutSwitch = checked
                    }

                    CheckBox {
                        Kirigami.FormData.label: i18n("Keyboard navigation:")
                        text: i18n("Show OSD when using keyboard navigation")
                        checked: kcm.showNavigationOsd
                        onToggled: kcm.showNavigationOsd = checked
                    }

                    ComboBox {
                        id: osdStyleCombo
                        Kirigami.FormData.label: i18n("OSD style:")
                        enabled: showOsdCheckbox.checked || kcm.showNavigationOsd

                        readonly property int osdStyleNone: 0
                        readonly property int osdStyleText: 1
                        readonly property int osdStylePreview: 2

                        currentIndex: Math.max(0, kcm.osdStyle - osdStyleText)
                        model: [
                            i18n("Text only"),
                            i18n("Visual preview")
                        ]
                        onActivated: (index) => {
                            kcm.osdStyle = index + osdStyleText
                        }
                    }
                }
            }
        }
    }
}
