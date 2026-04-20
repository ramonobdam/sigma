// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// ToolTip component with custom styling
ToolTip {
    id: control

    delay: Properties.toolTipDelay
    timeout: Properties.toolTipTimeout
    visible: false
    padding: Properties.spacingXS

    contentItem: SigmaText {
        text: control.text
        color: Properties.colorTextStrong
        textFormat: Text.StyledText
        wrapMode: Text.Wrap
    }

    background: Rectangle {
        color: Properties.colorElevated
        border.color: Properties.colorStrokeWeak
        radius: Properties.radiusS
        opacity: Properties.toolTipBackgroundOpacity
        layer.enabled: true
        layer.effect: RoundedElevationEffect {
            elevation: Properties.elevationElevated
            roundedScale: Material.ExtraSmallScale
        }
    }
}

