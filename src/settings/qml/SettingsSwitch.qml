// SPDX-FileCopyrightText: 2026 fuddlesworth
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtQuick.Controls
import org.kde.kirigami as Kirigami

Item {
    id: root

    property bool checked: false
    property string accessibleName: ""

    signal toggled()

    implicitWidth: 36
    implicitHeight: 18
    Accessible.role: Accessible.CheckBox
    Accessible.name: root.accessibleName
    Accessible.checked: root.checked

    // Track
    Rectangle {
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? Kirigami.Theme.highlightColor : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.2)

        // Knob
        Rectangle {
            width: 14
            height: 14
            radius: 7
            color: "white"
            border.color: Qt.rgba(0, 0, 0, 0.15)
            border.width: 0.5
            y: 2
            x: root.checked ? parent.width - width - 2 : 2

            Behavior on x {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutBack
                }

            }

        }

        Behavior on color {
            ColorAnimation {
                duration: 200
            }

        }

    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: root.toggled()
    }

}
