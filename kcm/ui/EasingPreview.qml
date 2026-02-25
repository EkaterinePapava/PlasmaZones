// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

/**
 * @brief Animated preview of an easing curve
 *
 * Shows a curve graph and an animated box that demonstrates the
 * currently selected easing function. Click anywhere to replay.
 */
Item {
    id: root

    property int easingCurveIndex: 2
    property int animationDuration: 150
    property bool previewEnabled: true

    implicitHeight: layout.implicitHeight
    implicitWidth: 280

    readonly property int graphHeight: 90
    readonly property int boxTrackHeight: 32
    readonly property int boxSize: 20

    // Map enum indices to QML easing types
    readonly property var easingTypes: [
        Easing.Linear,
        Easing.OutQuad,
        Easing.OutCubic,
        Easing.InOutCubic,
        Easing.OutBack,
        Easing.OutElastic,
        Easing.OutBounce,
        Easing.InOutBack,
        Easing.OutQuart,
        Easing.OutQuint,
        Easing.OutExpo,
        Easing.InOutQuad,
        Easing.InOutElastic
    ]

    // JS easing functions matching C++ WindowAnimation::applyEasing()
    function applyEasing(curve, t) {
        switch (curve) {
        case 0: return t
        case 1: return 1 - (1 - t) * (1 - t)
        case 2: return 1 - Math.pow(1 - t, 3)
        case 3: return t < 0.5 ? 4*t*t*t : 1 - Math.pow(-2*t + 2, 3) / 2
        case 4: {
            var c1 = 1.70158, c3 = c1 + 1
            return 1 + c3 * Math.pow(t-1, 3) + c1 * Math.pow(t-1, 2)
        }
        case 5: {
            if (t <= 0) return 0
            if (t >= 1) return 1
            var c4 = (2 * Math.PI) / 3
            return Math.pow(2, -10*t) * Math.sin((t*10 - 0.75) * c4) + 1
        }
        case 6: {
            if (t < 1/2.75) return 7.5625*t*t
            if (t < 2/2.75) { t -= 1.5/2.75; return 7.5625*t*t + 0.75 }
            if (t < 2.5/2.75) { t -= 2.25/2.75; return 7.5625*t*t + 0.9375 }
            t -= 2.625/2.75; return 7.5625*t*t + 0.984375
        }
        case 7: {
            var s1 = 1.70158, s2 = s1 * 1.525
            return t < 0.5
                ? (Math.pow(2*t, 2) * ((s2+1)*2*t - s2)) / 2
                : (Math.pow(2*t - 2, 2) * ((s2+1)*(t*2 - 2) + s2) + 2) / 2
        }
        case 8: return 1 - Math.pow(1 - t, 4)       // OutQuart
        case 9: return 1 - Math.pow(1 - t, 5)       // OutQuint
        case 10: return t >= 1 ? 1 : 1 - Math.pow(2, -10*t) // OutExpo
        case 11: return t < 0.5 ? 2*t*t : 1 - Math.pow(-2*t + 2, 2) / 2 // InOutQuad
        case 12: { // InOutElastic
            if (t <= 0) return 0
            if (t >= 1) return 1
            var c5 = (2 * Math.PI) / 4.5
            return t < 0.5
                ? -(Math.pow(2, 20*t - 10) * Math.sin((20*t - 11.125) * c5)) / 2
                :  (Math.pow(2, -20*t + 10) * Math.sin((20*t - 11.125) * c5)) / 2 + 1
        }
        default: return t
        }
    }

    onEasingCurveIndexChanged: replay()
    onAnimationDurationChanged: replay()

    function replay() {
        if (!root.previewEnabled) return
        previewAnim.stop()
        animBox.x = 0
        // Use at least 300ms for preview visibility regardless of actual setting
        previewAnim.duration = Math.max(root.animationDuration, 300)
        var idx = Math.max(0, Math.min(root.easingCurveIndex, root.easingTypes.length - 1))
        previewAnim.easing.type = root.easingTypes[idx]
        curveCanvas.requestPaint()
        replayDelay.restart()
    }

    Timer {
        id: replayDelay
        interval: 80
        onTriggered: previewAnim.start()
    }

    Component.onCompleted: Qt.callLater(replay)

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: 0

        // Curve graph canvas
        Canvas {
            id: curveCanvas
            Layout.fillWidth: true
            Layout.preferredHeight: root.graphHeight

            onPaint: {
                var ctx = getContext("2d")
                var w = width, h = height
                var pad = 12
                var gw = w - pad * 2, gh = h - pad * 2

                ctx.clearRect(0, 0, w, h)

                // Pre-scan the curve to find actual value range (for overshoot/elastic)
                var vMin = 0, vMax = 1
                var steps = 200
                for (var p = 0; p <= steps; p++) {
                    var val = root.applyEasing(root.easingCurveIndex, p / steps)
                    if (val < vMin) vMin = val
                    if (val > vMax) vMax = val
                }
                // Add small margin so the curve doesn't touch edges
                var margin = (vMax - vMin) * 0.08
                vMin -= margin
                vMax += margin
                var vRange = vMax - vMin

                // Helper: map a curve value to canvas Y coordinate
                function valToY(v) {
                    return pad + (1 - (v - vMin) / vRange) * gh
                }

                // Subtle background fill
                ctx.fillStyle = Qt.rgba(Kirigami.Theme.backgroundColor.r,
                                        Kirigami.Theme.backgroundColor.g,
                                        Kirigami.Theme.backgroundColor.b, 0.4)
                ctx.fillRect(0, 0, w, h)

                // Reference lines at y=0 and y=1 (solid, subtle)
                ctx.strokeStyle = Qt.rgba(Kirigami.Theme.disabledTextColor.r,
                                          Kirigami.Theme.disabledTextColor.g,
                                          Kirigami.Theme.disabledTextColor.b, 0.25)
                ctx.lineWidth = 1
                ctx.setLineDash([])
                var y0 = valToY(0), y1 = valToY(1)
                ctx.beginPath(); ctx.moveTo(pad, y0); ctx.lineTo(pad + gw, y0); ctx.stroke()
                ctx.beginPath(); ctx.moveTo(pad, y1); ctx.lineTo(pad + gw, y1); ctx.stroke()

                // Grid lines (dashed, evenly spaced across the visible range)
                ctx.strokeStyle = Qt.rgba(Kirigami.Theme.disabledTextColor.r,
                                          Kirigami.Theme.disabledTextColor.g,
                                          Kirigami.Theme.disabledTextColor.b, 0.1)
                ctx.setLineDash([2, 4])
                for (var j = 0; j <= 4; j++) {
                    var gx = pad + gw * j / 4
                    ctx.beginPath(); ctx.moveTo(gx, pad); ctx.lineTo(gx, pad + gh); ctx.stroke()
                }
                ctx.setLineDash([])

                // Linear reference (diagonal from (0,0) to (1,1))
                ctx.strokeStyle = Qt.rgba(Kirigami.Theme.disabledTextColor.r,
                                          Kirigami.Theme.disabledTextColor.g,
                                          Kirigami.Theme.disabledTextColor.b, 0.2)
                ctx.lineWidth = 1
                ctx.beginPath()
                ctx.moveTo(pad, y0)
                ctx.lineTo(pad + gw, y1)
                ctx.stroke()

                // Easing curve (highlighted)
                ctx.strokeStyle = Kirigami.Theme.highlightColor
                ctx.lineWidth = 2.5
                ctx.beginPath()
                var drawSteps = 150
                for (var s = 0; s <= drawSteps; s++) {
                    var t = s / drawSteps
                    var v = root.applyEasing(root.easingCurveIndex, t)
                    var px = pad + t * gw
                    var py = valToY(v)
                    if (s === 0) ctx.moveTo(px, py)
                    else ctx.lineTo(px, py)
                }
                ctx.stroke()

                // Axis labels
                ctx.fillStyle = Qt.rgba(Kirigami.Theme.textColor.r,
                                        Kirigami.Theme.textColor.g,
                                        Kirigami.Theme.textColor.b, 0.4)
                ctx.font = "9px sans-serif"
                ctx.textAlign = "right"
                ctx.fillText("0", pad - 3, y0 + 3)
                ctx.fillText("1", pad - 3, y1 + 3)
            }
        }

        // Animated box track
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: root.boxTrackHeight

            // Track rail
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: root.boxSize / 2
                anchors.rightMargin: root.boxSize / 2
                height: 3
                radius: 1.5
                color: Kirigami.Theme.separatorColor
            }

            // Start marker
            Rectangle {
                x: root.boxSize / 2 - 1
                anchors.verticalCenter: parent.verticalCenter
                width: 2; height: 12; radius: 1
                color: Kirigami.Theme.disabledTextColor
                opacity: 0.4
            }

            // End marker
            Rectangle {
                x: parent.width - root.boxSize / 2 - 1
                anchors.verticalCenter: parent.verticalCenter
                width: 2; height: 12; radius: 1
                color: Kirigami.Theme.disabledTextColor
                opacity: 0.4
            }

            // Animated box
            Rectangle {
                id: animBox
                width: root.boxSize
                height: root.boxSize
                radius: root.boxSize / 5
                y: (parent.height - height) / 2
                x: 0
                color: Kirigami.Theme.highlightColor

                NumberAnimation on x {
                    id: previewAnim
                    running: false
                    from: 0
                    to: curveCanvas.width - root.boxSize
                    duration: 300
                    easing.type: Easing.OutCubic
                }
            }

            // Click to replay
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                hoverEnabled: true
                onClicked: root.replay()

                ToolTip.visible: containsMouse
                ToolTip.delay: 500
                ToolTip.text: i18n("Click to replay animation preview")
            }
        }
    }
}
