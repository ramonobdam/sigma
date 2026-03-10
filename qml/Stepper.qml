// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

// Stepper component with custom styling
SpinBox {
    id: control

    property int inputFieldWidth: 30
    property int animationDuration: properties.animationDuration
    property string unacceptableInputMessage:
        properties.unacceptableInputValueText
    property bool showUnacceptableInputMessage: true   
    property string backgroundColor: properties.colorElevated
    property alias acceptableInput: textField.acceptableInput
    property alias text: textField.text

    editable: true
    stepSize: 1
    leftPadding: down.indicator.width -
                 ( acceptableInput ? properties.borderWidth : 0 )
    rightPadding: up.indicator.width -
                  ( acceptableInput ? properties.borderWidth : 0 )
    topPadding: 0
    bottomPadding: 0
    focus: true

    contentItem: SigmaTextField {
        id: textField

        text: control.textFromValue( control.value, control.locale )
        radius: 0
        dropShadow: false
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        showUnacceptableInputMessage: false
        KeyNavigation.backtab: control.KeyNavigation.backtab
        KeyNavigation.tab: control.KeyNavigation.tab
        KeyNavigation.left: KeyNavigation.backtab
        KeyNavigation.right: KeyNavigation.tab
        Keys.onDownPressed: control.decrease()
        Keys.onUpPressed: control.increase()
        Keys.onEnterPressed: control.Keys.onEnterPressed
    }

    up.indicator: RoundedRectangle {
        x: parent.width - width
        width: properties.textFieldHeight
        height: properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Right
        radius: bg.radius
        border.width: bg.border.width
        border.color:
            control.enabled ?
                (
                    control.up.pressed ?
                            properties.colorDown :
                            (
                                control.up.hovered ?
                                    properties.colorHover :
                                    properties.colorBrand
                            )
                ) :
                properties.colorStrokeWeak

        color:
            control.enabled ?
                (
                    control.up.pressed ?
                        properties.colorTextDown :
                        (
                            control.up.hovered ?
                                properties.colorTextHover :
                                control.backgroundColor
                        )
                ) :
                control.backgroundColor

        Behavior on color {
            ColorAnimation {
                duration: control.animationDuration
                easing.type: Easing.InOutQuad
            }
        }

        Text {
            anchors.fill: parent
            text: properties.plusIcon
            font.family: fonts.fontAwesome.font.family
            font.pixelSize: properties.fontSizeStepperIcon
            color: enabled ? parent.border.color : properties.colorStrokeWeak
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    down.indicator: RoundedRectangle {
        width: properties.textFieldHeight
        height: properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Left
        radius: bg.radius
        border.width: bg.border.width
        border.color:
            control.enabled ?
                (
                    control.down.pressed ?
                        properties.colorDown :
                            (
                                control.down.hovered ? properties.colorHover :
                                                       properties.colorBrand
                            )
                ) :
                properties.colorStrokeWeak

        color:
            control.enabled ?
                (
                    control.down.pressed ?
                        properties.colorTextDown :
                        (
                            control.down.hovered ? properties.colorTextHover :
                                                   control.backgroundColor
                        )
                ) :
                control.backgroundColor

        Behavior on color {
            ColorAnimation {
                duration: control.animationDuration
                easing.type: Easing.InOutQuad
            }
        }

        Text {
            anchors.fill: parent
            text: properties.minusIcon
            font.family: fonts.fontAwesome.font.family
            font.pixelSize: properties.fontSizeStepperIcon
            color: enabled ? parent.border.color : properties.colorStrokeWeak
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        id: bg
        implicitWidth: properties.textFieldHeight * 2 + control.inputFieldWidth
        border.width: properties.borderWidth
        border.color: control.enabled ? properties.colorBrand :
                                        properties.colorStrokeWeak
        color: control.backgroundColor
        radius: properties.radiusS

        // Apply dropshadow on stepper background when using lightTheme.
        layer.enabled: control.enabled && properties.lightTheme
        layer.effect: ButtonElevationEffect {}
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }

    Item {
        // Unacceptable input message
        anchors {
            top: control.top
            topMargin: ( control.height - icon.height ) / 2
            left: control.right
            leftMargin: properties.spacingM
        }

        visible: control.showUnacceptableInputMessage &&
                 !control.acceptableInput

        Rectangle {
            id: icon

            anchors {
                top: parent.top
                left: parent.left
            }
            height: errorText.height

            width: height
            radius: height / 2
            color: properties.colorTransparent
            border.width: properties.borderWidth
            border.color: properties.colorError500

            Text {
                anchors.fill: parent

                text: properties.invalidIcon
                font.family: fonts.fontAwesome.font.family
                font.pixelSize: properties.fontSizeUnacceptableInputIcon
                color: icon.border.color
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        SigmaText {
            id: errorText

            anchors {
                top: icon.top
                left: icon.right
                leftMargin: properties.spacingXS
            }

            text: control.unacceptableInputMessage
            color: icon.border.color
        }
    }
}
