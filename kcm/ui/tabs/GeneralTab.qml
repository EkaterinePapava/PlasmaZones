// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import ".."

/**
 * @brief General tab - Mode-agnostic settings (OSD, Animations)
 *
 * Contains settings that apply to both snapping and tiling modes,
 * such as on-screen display configuration and window animations.
 */
ScrollView {
    id: root

    required property var kcm
    required property QtObject constants

    clip: true
    contentWidth: availableWidth

    // Easing curve descriptions (indexed by EasingCurve enum)
    readonly property var easingDescriptions: [
        i18n("Constant speed from start to finish."),
        i18n("Quick start with gentle deceleration."),
        i18n("Smooth deceleration into place. Snappy and natural."),
        i18n("Smooth acceleration then deceleration."),
        i18n("Slight overshoot past the target, then settles back."),
        i18n("Spring-like bounce past the target."),
        i18n("Bounces at the end like a dropped ball."),
        i18n("Overshoots on both start and end."),
        i18n("Sharper deceleration than Cubic. Very responsive."),
        i18n("Even sharper deceleration. Ultra-snappy."),
        i18n("Exponential deceleration. Fastest non-overshoot curve."),
        i18n("Gentle symmetric acceleration and deceleration."),
        i18n("Elastic spring on both ends. Very expressive.")
    ]

    ColumnLayout {
        width: parent.width
        spacing: Kirigami.Units.largeSpacing

        // ═══════════════════════════════════════════════════════════════════════
        // ANIMATIONS CARD
        // ═══════════════════════════════════════════════════════════════════════
        Item {
            Layout.fillWidth: true
            implicitHeight: animationsCard.implicitHeight

            Kirigami.Card {
                id: animationsCard
                anchors.fill: parent

                header: Kirigami.Heading {
                    level: 3
                    text: i18n("Animations")
                    padding: Kirigami.Units.smallSpacing
                }

                contentItem: ColumnLayout {
                    spacing: Kirigami.Units.largeSpacing

                    // Enable toggle
                    CheckBox {
                        id: animationsEnabledCheck
                        Layout.fillWidth: true
                        text: i18n("Smooth window geometry transitions")
                        checked: kcm.animationsEnabled
                        onToggled: kcm.animationsEnabled = checked

                        ToolTip.visible: hovered
                        ToolTip.text: i18n("Animate windows when snapping to zones or tiling. Applies to both manual snapping and autotiling.")
                    }

                    // Easing curve preview (visual)
                    EasingPreview {
                        id: easingPreview
                        Layout.fillWidth: true
                        Layout.maximumWidth: 320
                        Layout.alignment: Qt.AlignHCenter
                        easingCurveIndex: kcm.animationEasingCurve
                        animationDuration: kcm.animationDuration
                        previewEnabled: animationsEnabledCheck.checked
                        opacity: animationsEnabledCheck.checked ? 1.0 : 0.4
                    }

                    // Replay button
                    Button {
                        Layout.alignment: Qt.AlignHCenter
                        text: i18n("Replay Preview")
                        icon.name: "media-playback-start"
                        enabled: animationsEnabledCheck.checked
                        onClicked: easingPreview.replay()
                        flat: true
                    }

                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }

                    Kirigami.FormLayout {
                        Layout.fillWidth: true

                        // Easing curve selector
                        ComboBox {
                            id: easingCurveCombo
                            Kirigami.FormData.label: i18n("Easing curve:")
                            enabled: animationsEnabledCheck.checked

                            currentIndex: kcm.animationEasingCurve
                            model: [
                                i18n("Linear"),
                                i18n("Ease Out (Quad)"),
                                i18n("Ease Out (Cubic)"),
                                i18n("Ease In-Out (Cubic)"),
                                i18n("Overshoot"),
                                i18n("Elastic"),
                                i18n("Bounce"),
                                i18n("Overshoot (In-Out)"),
                                i18n("Ease Out (Quart)"),
                                i18n("Ease Out (Quint)"),
                                i18n("Ease Out (Expo)"),
                                i18n("Ease In-Out (Quad)"),
                                i18n("Elastic (In-Out)")
                            ]
                            onActivated: (index) => {
                                kcm.animationEasingCurve = index
                            }

                            ToolTip.visible: hovered
                            ToolTip.text: i18n("Controls the acceleration curve of the animation")
                        }

                        // Easing description
                        Label {
                            Kirigami.FormData.label: " "
                            Layout.fillWidth: true
                            text: root.easingDescriptions[kcm.animationEasingCurve] || ""
                            wrapMode: Text.WordWrap
                            opacity: animationsEnabledCheck.checked ? 0.7 : 0.3
                            font.italic: true
                        }

                        // Duration slider
                        RowLayout {
                            Kirigami.FormData.label: i18n("Duration:")
                            enabled: animationsEnabledCheck.checked
                            spacing: Kirigami.Units.smallSpacing

                            Slider {
                                id: animationDurationSlider
                                Layout.preferredWidth: root.constants.sliderPreferredWidth
                                from: 50
                                to: 500
                                stepSize: 10
                                value: kcm.animationDuration
                                onMoved: kcm.animationDuration = Math.round(value)

                                ToolTip.visible: hovered
                                ToolTip.text: i18n("How long window animations take to complete (milliseconds)")
                            }

                            Label {
                                text: Math.round(animationDurationSlider.value) + " ms"
                                Layout.preferredWidth: root.constants.sliderValueLabelWidth + 15
                            }
                        }

                        // Minimum distance threshold
                        RowLayout {
                            Kirigami.FormData.label: i18n("Min. distance:")
                            enabled: animationsEnabledCheck.checked
                            spacing: Kirigami.Units.smallSpacing

                            SpinBox {
                                from: 0
                                to: 200
                                stepSize: 5
                                value: kcm.animationMinDistance
                                onValueModified: kcm.animationMinDistance = value

                                ToolTip.visible: hovered
                                ToolTip.text: i18n("Skip animation when the geometry change is smaller than this many pixels. Prevents jittery micro-animations.")
                            }

                            Label {
                                text: i18n("px")
                            }

                            Label {
                                text: kcm.animationMinDistance === 0 ? i18n("(always animate)") : ""
                                opacity: 0.6
                                font.italic: true
                            }
                        }
                    }
                }
            }
        }

        // ═══════════════════════════════════════════════════════════════════════
        // ON-SCREEN DISPLAY CARD
        // ═══════════════════════════════════════════════════════════════════════
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
