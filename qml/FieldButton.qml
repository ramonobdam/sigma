// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// Button component used in combination with an input field.
Button {
    id: button

    property string backgroundColor: Properties.colorElevated
    property string buttonIcon: Properties.symbolIcon
    property var control

    height: Properties.buttonHeight
    width: height
    padding: 0
    topInset: 0
    bottomInset: 0
    hoverEnabled: true
    checkable: true
    text: checked ? Properties.crossIcon : button.buttonIcon
    font.family: checked ? Fonts.fontAwesome.font.family :
                           Fonts.interSemiBold.font.family
    font.pixelSize: checked ? Properties.fontSizeComboBoxIcon :
                              Properties.fontSizeFieldButtonIcon
    font.bold: true

    contentItem: Text {
        id: text

        anchors.fill: parent

        text: button.text
        font: button.font
        color: bg.border.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: Properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    background: RoundedRectangle {
        id: bg
        anchors.fill: parent
        height: Properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Left
        radius: Properties.radiusS
        border.width: Properties.borderWidth
        border.color:
            button.enabled ? ( button.pressed ?
                                  Properties.colorDown :
                                  (
                                      button.hovered ? Properties.colorHover :
                                                       Properties.colorBrand
                                  )
                              ) :
                              Properties.colorStrokeWeak

        color:
            button.enabled ? ( button.pressed ?
                                  Properties.colorTextDown :
                                  (
                                      button.hovered ?
                                          Properties.colorTextHover :
                                          button.backgroundColor
                                  )
                             ) :
                             button.backgroundColor

        // Apply dropshadow on background when using lightTheme.
        layer.enabled: button.enabled && Properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Behavior on color {
            ColorAnimation {
                duration: Properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
}
