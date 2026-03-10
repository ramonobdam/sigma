// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// BusyIndicator with customized styling
BusyIndicator {
    id: control

    contentItem: BusyIndicatorImpl {
        implicitWidth: properties.busyIndicatorWidth
        implicitHeight: properties.busyIndicatorWidth
        color: properties.colorBrand

        running: control.running
        opacity: control.running ? 1 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: properties.busyIndicatorOpacityDuration
            }
        }
    }

    SigmaProperties {
        id: properties
    }
}
