// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Effects

// Menu component with custom styling for the menu bar
Menu {
    id: control

    implicitWidth: properties.contextMenuWidth
    padding: bg.border.width
    verticalPadding: bg.border.width

    background: Rectangle {
        id: bg
        color: properties.colorElevated
        radius: properties.radiusS
        border.width: properties.borderWidth
        border.color: properties.colorStrokeWeak
        layer.enabled: true
        layer.effect: RoundedElevationEffect {
            elevation: properties.elevationElevated
            roundedScale: Material.ExtraSmallScale
        }
    }

    contentItem: ListView {
        id: listView

        implicitHeight: contentHeight
        model: control.contentModel
        interactive: Window.window ?
                         (
                            (
                                contentHeight +
                                control.topPadding +
                                control.bottomPadding
                            ) > control.height
                         ) :
                         false
        clip: true
        currentIndex: control.currentIndex

        ScrollIndicator.vertical: SigmaScrollIndicator {}

        // Clip the rounded corners of the background by using a MultiEffect
        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: mask
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
        }

        Rectangle {
            id: mask
            width: listView.width
            height: listView.height
            radius: bg.radius - bg.border.width
            visible: false
            layer.enabled: true
            layer.smooth: true
        }
    }

    SigmaProperties {
        id: properties
    }
}
