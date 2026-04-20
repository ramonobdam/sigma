// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Sigma

// Component used for the Monte Carlo results table left column (i.e. header)
Item {
    id: control

    property string text

    anchors {
        left: parent.left
        right: parent.right
        rightMargin: -( Properties.spacingM ) / 2
    }
    height: Properties.rowHeight + line.height

    Rectangle {
        id: separatorLeft

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: ( Properties.rowHeight - Properties.separatorHeight ) / 2
            bottomMargin:
                ( Properties.rowHeight - Properties.separatorHeight ) / 2 +
                line.height
        }
        width: Properties.borderWidth

        color: Properties.colorStrokeWeak
        antialiasing: true
    }

    Rectangle {
        id: separatorRight

        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            topMargin: ( Properties.rowHeight - Properties.separatorHeight ) / 2
            bottomMargin:
                ( Properties.rowHeight - Properties.separatorHeight ) / 2 +
                line.height

        }
        width: Properties.borderWidth

        color: Properties.colorStrokeWeak
        antialiasing: true
    }

    SigmaText {
        anchors {
            left: parent.left
            right: separatorRight.left
            top: parent.top
            bottom: line.top
        }

        text: control.text
        color: Properties.colorTextStrong
        padding: Properties.spacingS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: Properties.borderWidth

        color: Properties.colorStrokeWeak
        antialiasing: true
    }
}
