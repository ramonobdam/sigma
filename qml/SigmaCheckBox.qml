// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import Sigma

// CheckBox component with custom styling
CheckBox {
    id: control

    property string backgroundColor: Properties.colorElevated
    property string primaryColor:
        enabled ? (
                       down ? Properties.colorDown :
                              (
                                   hovered ? Properties.colorHover :
                                             Properties.colorBrand
                              )
                  ) :
                  Properties.colorStrokeWeak
    property string secondaryColor:
        enabled ? (
                      down ? Properties.colorTextDown :
                             (
                                 hovered ? Properties.colorTextHover :
                                           control.backgroundColor
                             )
                  ) :
                  control.backgroundColor

    height: indicator.height
    focus: true

    KeyNavigation.left: KeyNavigation.backtab
    KeyNavigation.right: KeyNavigation.tab
    KeyNavigation.up: KeyNavigation.backtab
    KeyNavigation.down: KeyNavigation.tab
    Keys.onReturnPressed: toggle()

    indicator: Rectangle {
        id: indicator

        width: Properties.checkBoxWidth
        height: Properties.checkBoxWidth
        radius: Properties.radiusS + Properties.borderWidth * 2
        border.width: control.activeFocus ? Properties.borderWidth : 0
        border.color: bg.border.color

        // Apply dropshadow on 'activeFocus border' when using lightTheme.
        // The background is filled instead of transparent to only show the
        // shadow around the 'activeFocus border'.
        color: Properties.lightTheme ? control.backgroundColor :
                                       Properties.colorTransparent
        layer.enabled: control.activeFocus &&
                       control.enabled &&
                       Properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Rectangle {
            id: bg

            anchors {
                fill: parent
                margins: Properties.borderWidth * 2
            }

            color: control.checked ? control.primaryColor :
                                     control.secondaryColor
            border.color: primaryColor
            border.width: Properties.borderWidth
            radius: Properties.radiusS

            // Apply dropshadow on button background when using lightTheme.
            layer.enabled: !control.activeFocus &&
                           control.enabled &&
                           Properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            Behavior on color {
                ColorAnimation {
                    duration: Properties.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Text {
                anchors.fill: parent

                visible: control.checked
                text: Properties.validIcon
                font.family: Fonts.fontAwesome.font.family
                font.pixelSize: Properties.fontSizeCheckBoxIcon
                color: control.backgroundColor
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }

    contentItem: SigmaText {
        id: text

        height: control.height
        width: text.contentWidth + text.leftPadding
        text: control.text
        color: Properties.colorTextStrong
        leftPadding: indicator.width + Properties.spacingXS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
