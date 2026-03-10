// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick

// Delegate component for table headers with an optional valid/invalid icon on
// the left side
Item {
    id: control

    required property int column
    required property string display
    property bool useValid: false
    property bool lastColumn: column === ( TableView.view.columns - 1 )
    property var columnWidthProvider:
        function( column ) { return properties.defaultTableWidth }

    implicitWidth: columnWidthProvider( column )
    implicitHeight: properties.rowHeight + line.height
    clip: true

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }

    Rectangle {
        id: separatorLeft

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: ( properties.rowHeight - properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height

        }
        width: properties.borderWidth

        color: properties.colorStrokeWeak
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
                   ( properties.validIconWidth - separatorLeft.width ) :
                   0

        visible: control.useValid
        text: properties.validIcon
        font.family: fonts.fontAwesome.font.family
        font.pixelSize: properties.fontSizeValidIcon
        color: properties.colorTextStrong
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    Rectangle {
        id: iconSeparator

        anchors {
            left: icon.right
            top: parent.top
            bottom: parent.bottom
            topMargin: ( properties.rowHeight - properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height
        }
        width: control.useValid ? properties.borderWidth : 0

        visible: control.useValid
        color: properties.colorStrokeWeak
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
        color: properties.colorTextStrong
        padding: properties.spacingS
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
            topMargin: ( properties.rowHeight - properties.separatorHeight ) / 2
            bottomMargin: anchors.topMargin + line.height
        }
        width: control.lastColumn ? properties.borderWidth : 0

        visible: control.lastColumn
        color: properties.colorStrokeWeak
        antialiasing: true
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: properties.borderWidth

        color: properties.colorStrokeWeak
        antialiasing: true
    }
}

