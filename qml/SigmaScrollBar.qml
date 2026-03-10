// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// ScrollBar component with custom styling
ScrollBar {
    id: control

    property int indicatorWidth: properties.scrollBarHeight

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
        color: control.pressed ? properties.colorDown :
                                 (
                                     control.hovered ? properties.colorHover :
                                                       properties.colorBrand
                                 )

        Behavior on color {
            ColorAnimation {
                duration: properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    background: Rectangle {
        color: properties.colorFill
        radius: control.indicatorWidth / 2
    }

    SigmaProperties {
        id: properties
    }
}
