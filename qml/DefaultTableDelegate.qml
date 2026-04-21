// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Table delegate with optional valid icon on the left and color scale on the
// right
TableDelegate {
    id: control

    property bool useScale: false
    property bool useValid: false
    property string icon: decoration ? Properties.validIcon :
                                       Properties.invalidIcon
    property string validColor: decoration ? Properties.colorSuccess500 :
                                             Properties.colorError500
    required property int decoration

    Rectangle {
        id: validRectangle

        anchors {
            left: parent.left
            top: parent.top
            bottom: line.top
        }
        width: control.useValid ? Properties.validIconWidth : 0
        visible: control.useValid
        color: control.validColor

        Text {
            anchors.fill: parent

            text: control.icon
            font.family: Fonts.fontAwesome.font.family
            font.pixelSize: Properties.fontSizeValidIcon
            color: control.backgroundColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Rectangle {
        id: separatorLeft

        anchors {
            left: validRectangle.right
            top: parent.top
            bottom: line.top
        }
        width: control.useValid ? Properties.borderWidth : 0

        visible: control.useValid
        color: control.backgroundColor
        antialiasing: true
    }

    Rectangle {
        id: textBox

        anchors {
            left: separatorLeft.right
            right: separatorRight.left
            top: parent.top
            bottom: line.top
            margins: Properties.spacingXXS
        }

        color: Properties.colorTransparent
        radius: Properties.radiusS
        border.width: control.hasFocus ? Properties.borderWidth : 0
        border.color: control.backgroundColor

        SigmaText {
            anchors.fill: parent

            text: control.display
            color: control.currentRow ? control.backgroundColor :
                                        Properties.colorTextWeak
            textFormat: Text.PlainText
            elide: Text.ElideRight
            padding: Properties.spacingS - textBox.anchors.margins
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
    }

    Rectangle {
        id: separatorRight

        anchors {
            left: scaleRectangle.left
            top: parent.top
            bottom: line.top
        }
        width: control.useScale ? Properties.borderWidth : 0

        visible: control.useScale
        color: control.backgroundColor
        antialiasing: true
    }

    Rectangle {
        id: scaleRectangle

        anchors {
            right: parent.right
            top: parent.top
            bottom: line.top
        }
        width: control.useScale ? Properties.colorScaleWidth : 0

        visible: control.useScale
        color: (
                    control.useScale &&
                    control.display != "-" &&
                    control.decoration >= 0 &&
                    control.decoration < Properties.colorScale.length
               ) ?
                    Properties.colorScale[ control.decoration ] :
                    Properties.colorTransparent
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
