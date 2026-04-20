// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Delegate component for table headers with an optional valid/invalid icon on
// the left side
Item {
    id: control

    required property int column
    required property string display
    property bool useValid: false
    property bool lastColumn: column === ( TableView.view.columns - 1 )
    property var columnWidthProvider:
        function( column ) { return Properties.defaultTableWidth }

    implicitWidth: columnWidthProvider( column )
    implicitHeight: Properties.rowHeight + line.height
    clip: true

    Rectangle {
        id: separatorLeft

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: ( Properties.rowHeight - Properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height

        }
        width: Properties.borderWidth

        color: Properties.colorStrokeWeak
        antialiasing: true
    }

    Text {
        id: icon

        anchors {
            left: separatorLeft.right
            top: parent.top
            bottom: line.top
        }
        width: control.useValid ?
                   ( Properties.validIconWidth - separatorLeft.width ) :
                   0

        visible: control.useValid
        text: Properties.validIcon
        font.family: Fonts.fontAwesome.font.family
        font.pixelSize: Properties.fontSizeValidIcon
        color: Properties.colorTextStrong
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    Rectangle {
        id: iconSeparator

        anchors {
            left: icon.right
            top: parent.top
            bottom: parent.bottom
            topMargin: ( Properties.rowHeight - Properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height
        }
        width: control.useValid ? Properties.borderWidth : 0

        visible: control.useValid
        color: Properties.colorStrokeWeak
        antialiasing: true
    }

    SigmaText {
        anchors {
            left: iconSeparator.right
            right: separatorRight.left
            top: parent.top
            bottom: line.top
        }

        text: control.display
        color: Properties.colorTextStrong
        padding: Properties.spacingS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        id: separatorRight

        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            topMargin: ( Properties.rowHeight - Properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height
        }
        width: control.lastColumn ? Properties.borderWidth : 0

        visible: control.lastColumn
        color: Properties.colorStrokeWeak
        antialiasing: true
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

