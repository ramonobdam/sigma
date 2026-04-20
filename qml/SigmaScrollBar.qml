// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Sigma

// ScrollBar component with custom styling
ScrollBar {
    id: control

    property int indicatorWidth: Properties.scrollBarHeight

    width: indicatorWidth
    height: indicatorWidth
    visible: size < 1

    // Disable Material defined states and make the ScrollBar always active.
    state: ""
    states: []

    contentItem: Rectangle {
        width: control.vertical ? control.indicatorWidth : 0
        height: control.horizontal ? control.indicatorWidth : 0
        radius: control.indicatorWidth / 2
        color: control.pressed ? Properties.colorDown :
                                 (
                                     control.hovered ? Properties.colorHover :
                                                       Properties.colorBrand
                                 )

        Behavior on color {
            ColorAnimation {
                duration: Properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    background: Rectangle {
        color: Properties.colorFill
        radius: control.indicatorWidth / 2
    }
}
