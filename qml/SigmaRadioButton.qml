// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

// RadioButton component with custom styling
RadioButton {
    id: button

    property int animationDuration: properties.animationDuration
    property string backgroundColor: properties.colorElevated

    property string primaryColor:
        enabled ? ( down ? properties.colorDown :
                           (
                                hovered ? properties.colorHover :
                                          properties.colorBrand
                            )
                   ) :
                    properties.colorStrokeWeak
    property string secondaryColor:
        enabled ? ( down ? properties.colorTextDown :
                           (
                               hovered ? properties.colorTextHover :
                                         backgroundColor
                           )
                   ) :
                   backgroundColor

    height: indicator.height
    focus: true

    KeyNavigation.left: KeyNavigation.backtab
    KeyNavigation.right: KeyNavigation.tab
    KeyNavigation.up: KeyNavigation.backtab
    KeyNavigation.down: KeyNavigation.tab
    Keys.onReturnPressed: toggle()

    indicator: Rectangle {
        implicitWidth: properties.radioButtonWidth
        implicitHeight: properties.radioButtonWidth
        radius: properties.radioButtonWidth / 2
        border.width: button.activeFocus ? properties.borderWidth : 0
        border.color: bg.border.color

        // Apply dropshadow on 'activeFocus border' when using lightTheme.
        // The background is filled instead of transparent to only show the
        // shadow around the 'activeFocus border'.
        color: properties.lightTheme ? backgroundColor :
                                       properties.colorTransparent
        layer.enabled: button.activeFocus &&
                       button.enabled &&
                       properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Rectangle {
            id: bg

            anchors {
                fill: parent
                margins: properties.borderWidth * 2
            }

            color: secondaryColor
            border.color: primaryColor
            border.width: properties.borderWidth
            radius: parent.radius - properties.borderWidth * 2

            // Apply dropshadow on button background when using lightTheme
            layer.enabled: !button.activeFocus &&
                           button.enabled &&
                           properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            Behavior on color {
                ColorAnimation {
                    duration: button.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: properties.radioButtonCheckWidth
                height: width
                radius: width / 2
                color: bg.border.color
                visible: button.checked
            }
        }
    }

    contentItem: SigmaText {
        id: text

        text: button.text
        color: properties.colorTextStrong
        height: button.height
        width: text.contentWidth + text.leftPadding
        leftPadding: indicator.width + properties.spacingXS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }

    SigmaProperties {
        id: properties
    }
}
