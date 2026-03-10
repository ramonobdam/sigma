// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// MenuBarItem with custom styling
MenuBarItem {
    id: control

    property int animationDuration: properties.animationDuration
    property string backgroundColor: properties.colorElevated

    implicitHeight: properties.menuBarHeight
    spacing: 0
    padding: 0
    leftPadding: 0
    rightPadding: 0

    anchors.verticalCenter: parent.verticalCenter

    contentItem: SigmaText {
        anchors.fill: parent
        text: control.text
        font.pixelSize: properties.fontSizeTitleBar
        color: control.enabled ? properties.colorTextStrong :
                                 properties.colorStrokeStrong
        elide: Text.ElideRight
        leftPadding: properties.spacingS
        rightPadding: properties.spacingS
        topPadding: properties.spacingXXS
        bottomPadding: properties.spacingXXS
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        color: control.highlighted ? properties.colorTextHover :
                                     control.backgroundColor
        radius: properties.radiusS

        Behavior on color {
            ColorAnimation {
                duration: control.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    SigmaProperties {
        id: properties
    }
}
