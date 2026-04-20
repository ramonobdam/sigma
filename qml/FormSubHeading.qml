// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Sub heading used in forms with a text and sub text
Item {
    id: control

    property alias text: label.text
    property alias subText: subLabel.text

    anchors {
        left: parent.left
        right: parent.right
        topMargin: Properties.spacingL
    }
    height: label.height +
            line.anchors.topMargin +
            line.height +
            subLabel.anchors.topMargin +
            subLabel.height

    SigmaText {
        id: label

        anchors {
            left: parent.left
            right: parent.right
        }
        height: text.length > 0 ? contentHeight : 0

        color: Properties.colorTextStrong
        elide: Text.ElideRight
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            top: label.bottom
            topMargin: control.text.length > 0 ? Properties.spacingXS : 0
        }
        height: Properties.borderWidth

        color: Properties.colorStrokeWeak
        antialiasing: true
    }

    SigmaText {
        id: subLabel

        anchors {
            left: parent.left
            right: parent.right
            top: line.bottom
            topMargin: height > 0 ? Properties.spacingS : 0
        }
        height: text.length > 0 ? contentHeight : 0

        color: Properties.colorTextWeak
        elide: Text.ElideRight
    }
}
