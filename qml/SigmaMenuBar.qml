// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// MenuBar component with custom styling
MenuBar {
    id: control

    property string backgroundColor: properties.colorBase

    anchors.verticalCenter: parent.verticalCenter

    implicitWidth: Math.max(
                       implicitBackgroundWidth + leftInset + rightInset,
                       implicitContentWidth + leftPadding + rightPadding
                    )
    implicitHeight: properties.menuBarHeight

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0
    spacing: 0

    delegate: SigmaMenuBarItem {
        backgroundColor: control.backgroundColor
    }

    contentItem: Row {
        spacing: control.spacing
        Repeater {
            model: control.contentModel
        }
    }

    background: Rectangle {
        color: control.backgroundColor
    }

    SigmaProperties {
        id: properties
    }

}
