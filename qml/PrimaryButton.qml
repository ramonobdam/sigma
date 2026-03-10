// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// Button component with the highest prominence in the visual hierarchy
Button {
    id: button

    property int defaultWidth: contentItem.implicitWidth + padding * 2
    property string toolTipText
    property string toolTipTextDisabled: toolTipText
    property string backgroundColor: properties.colorElevated
    property alias textColor: text.color
    property alias fillColor: bg.color
    property alias borderColor: bg.border.color

    height: visible ? ( properties.buttonHeight + properties.borderWidth * 4 ) :
                      0
    width: defaultWidth < height ? height : defaultWidth
    padding: properties.spacingM
    focus: true
    hoverEnabled: true

    KeyNavigation.left: KeyNavigation.backtab
    KeyNavigation.right: KeyNavigation.tab
    Keys.onReturnPressed: clicked()

    // Disable ContextMenu on button, in case the parent has one defined
    ContextMenu.onRequested: {}

    contentItem: SigmaText {
        id: text

        anchors.fill: parent

        text: button.text
        font.family: fonts.interSemiBold.font.family
        font.bold: true
        color: button.enabled ? (
                                    button.down ?
                                        properties.colorTextDown :
                                            (
                                                button.hovered ?
                                                    properties.colorTextHover :
                                                    backgroundColor
                                            )
                                ) :
                                properties.colorStrokeStrong
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent

        radius: properties.radiusS + properties.borderWidth * 2
        border.width: button.activeFocus ? properties.borderWidth : 0
        border.color: bg.border.color

        // Apply dropshadow on 'activeFocus border' when using lightTheme.
        // The background is filled instead of transparent to only show the
        // shadow around the 'activeFocus border'.
        color: backgroundColor
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
            border.width: properties.borderWidth
            border.color: bg.color
            color: button.enabled ? (
                                        button.down ?
                                            properties.colorDown :
                                            (
                                                button.hovered ?
                                                    properties.colorHover :
                                                    properties.colorBrand
                                            )
                                    ) :
                                    properties.colorStrokeWeak
            radius: properties.radiusS

            // Apply dropshadow on button background when using lightTheme.
            layer.enabled: !button.activeFocus &&
                           button.enabled &&
                           properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            Behavior on color {
                ColorAnimation {
                    duration: properties.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }

    SigmaToolTip {
        id: control

        visible: button.hovered && button.toolTipText.length > 0
        text: button.enabled ? button.toolTipText : button.toolTipTextDisabled
    }
}
