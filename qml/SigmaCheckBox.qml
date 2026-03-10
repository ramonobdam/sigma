// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

// CheckBox component with custom styling
CheckBox {
    id: control

    property string backgroundColor: properties.colorElevated
    property string primaryColor:
        enabled ? (
                       down ? properties.colorDown :
                              (
                                   hovered ? properties.colorHover :
                                             properties.colorBrand
                              )
                  ) :
                  properties.colorStrokeWeak
    property string secondaryColor:
        enabled ? (
                      down ? properties.colorTextDown :
                             (
                                 hovered ? properties.colorTextHover :
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

        width: properties.checkBoxWidth
        height: properties.checkBoxWidth
        radius: properties.radiusS + properties.borderWidth * 2
        border.width: control.activeFocus ? properties.borderWidth : 0
        border.color: bg.border.color

        // Apply dropshadow on 'activeFocus border' when using lightTheme.
        // The background is filled instead of transparent to only show the
        // shadow around the 'activeFocus border'.
        color: properties.lightTheme ? control.backgroundColor :
                                       properties.colorTransparent
        layer.enabled: control.activeFocus &&
                       control.enabled &&
                       properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Rectangle {
            id: bg

            anchors {
                fill: parent
                margins: properties.borderWidth * 2
            }

            color: control.checked ? control.primaryColor :
                                     control.secondaryColor
            border.color: primaryColor
            border.width: properties.borderWidth
            radius: properties.radiusS

            // Apply dropshadow on button background when using lightTheme.
            layer.enabled: !control.activeFocus &&
                           control.enabled &&
                           properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            Behavior on color {
                ColorAnimation {
                    duration: properties.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Text {
                anchors.fill: parent

                visible: control.checked
                text: properties.validIcon
                font.family: fonts.fontAwesome.font.family
                font.pixelSize: properties.fontSizeCheckBoxIcon
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
        color: properties.colorTextStrong
        leftPadding: indicator.width + properties.spacingXS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
