// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// Styled horizontal rule component for (context) menus
MenuSeparator {
    id: control

    padding: 0
    verticalPadding: 0

    contentItem: Rectangle {
        implicitHeight: visible ? properties.borderWidth : 0
        color: properties.colorStrokeWeak
        antialiasing: true
    }

    SigmaProperties {
        id: properties
    }
}
