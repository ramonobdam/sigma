// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import Sigma

// RadioButton component with custom styling
RadioButton {
    id: button

    property int animationDuration: Properties.animationDuration
    property string backgroundColor: Properties.colorElevated

    property string primaryColor:
        enabled ? ( down ? Properties.colorDown :
                           (
                                hovered ? Properties.colorHover :
                                          Properties.colorBrand
                            )
                   ) :
                    Properties.colorStrokeWeak
    property string secondaryColor:
        enabled ? ( down ? Properties.colorTextDown :
                           (
                               hovered ? Properties.colorTextHover :
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
    Keys.onReturnPressed: { click() }

    indicator: Rectangle {
        implicitWidth: Properties.radioButtonWidth
        implicitHeight: Properties.radioButtonWidth
        radius: Properties.radioButtonWidth / 2
        border.width: button.activeFocus ? Properties.borderWidth : 0
        border.color: bg.border.color

        // Apply dropshadow on 'activeFocus border' when using lightTheme.
        // The background is filled instead of transparent to only show the
        // shadow around the 'activeFocus border'.
        color: Properties.lightTheme ? backgroundColor :
                                       Properties.colorTransparent
        layer.enabled: button.activeFocus &&
                       button.enabled &&
                       Properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Rectangle {
            id: bg

            anchors {
                fill: parent
                margins: Properties.borderWidth * 2
            }

            color: secondaryColor
            border.color: primaryColor
            border.width: Properties.borderWidth
            radius: parent.radius - Properties.borderWidth * 2

            // Apply dropshadow on button background when using lightTheme
            layer.enabled: !button.activeFocus &&
                           button.enabled &&
                           Properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            Behavior on color {
                ColorAnimation {
                    duration: button.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: Properties.radioButtonCheckWidth
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
        color: Properties.colorTextStrong
        height: button.height
        width: text.contentWidth + text.leftPadding
        leftPadding: indicator.width + Properties.spacingXS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
    }
}
