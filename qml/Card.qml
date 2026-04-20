// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// Styled card component with rounded corners, background, border and heading
Pane {
    id: control

    property string title
    property alias header: header

    anchors {
        fill: parent
    }
    padding: Properties.spacingM
    background: Rectangle {
        color: Properties.colorRaised
        radius: Properties.radiusM
        border.width: Properties.lightTheme ? 0 : Properties.borderWidth
        border.color: Properties.colorStrokeWeak
        layer.enabled: Properties.lightTheme
        layer.effect: RoundedElevationEffect {
            elevation: Properties.elevationRaised
            roundedScale: Material.ExtraSmallScale
        }
    }


    Item {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: heading.height + Properties.spacingM

        Heading {
            id: heading
            text: control.title
        }
    }
}
