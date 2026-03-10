// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// ToolTip component with custom styling
ToolTip {
    id: control

    delay: properties.toolTipDelay
    timeout: properties.toolTipTimeout
    visible: false
    padding: properties.spacingXS

    contentItem: SigmaText {
        text: control.text
        color: properties.colorTextStrong
        textFormat: Text.StyledText
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        color: properties.colorElevated
        border.color: properties.colorStrokeWeak
        radius: properties.radiusS
        opacity: properties.toolTipBackgroundOpacity
        layer.enabled: true
        layer.effect: RoundedElevationEffect {
            elevation: properties.elevationElevated
            roundedScale: Material.ExtraSmallScale
        }
    }

    SigmaProperties {
        id: properties
    }
}

