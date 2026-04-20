// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import Sigma

// MenuBarItem with custom styling
MenuBarItem {
    id: control

    property int animationDuration: Properties.animationDuration
    property string backgroundColor: Properties.colorElevated

    implicitHeight: Properties.menuBarHeight
    spacing: 0
    padding: 0
    leftPadding: 0
    rightPadding: 0

    anchors.verticalCenter: parent.verticalCenter

    contentItem: SigmaText {
        anchors.fill: parent
        text: control.text
        font.pixelSize: Properties.fontSizeTitleBar
        color: control.enabled ? Properties.colorTextStrong :
                                 Properties.colorStrokeStrong
        elide: Text.ElideRight
        leftPadding: Properties.spacingS
        rightPadding: Properties.spacingS
        topPadding: Properties.spacingXXS
        bottomPadding: Properties.spacingXXS
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.highlighted ? Properties.colorTextHover :
                                     control.backgroundColor
        radius: Properties.radiusS

        Behavior on color {
            ColorAnimation {
                duration: control.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}
