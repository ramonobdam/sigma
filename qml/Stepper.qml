// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal
import Sigma

// Stepper component with custom styling
SpinBox {
    id: control

    property int inputFieldWidth: 30
    property int animationDuration: Properties.animationDuration
    property string unacceptableInputMessage:
        Properties.unacceptableInputValueText
    property bool showUnacceptableInputMessage: true   
    property string backgroundColor: Properties.colorElevated
    property alias acceptableInput: textField.acceptableInput
    property alias text: textField.text

    editable: true
    stepSize: 1
    leftPadding: down.indicator.width -
                 ( acceptableInput ? Properties.borderWidth : 0 )
    rightPadding: up.indicator.width -
                  ( acceptableInput ? Properties.borderWidth : 0 )
    topPadding: 0
    bottomPadding: 0
    focus: true

    contentItem: SigmaTextField {
        id: textField

        implicitWidth: control.inputFieldWidth
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
        width: Properties.textFieldHeight
        height: Properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Right
        radius: bg.radius
        border.width: bg.border.width
        border.color:
            control.enabled ?
                (
                    control.up.pressed ?
                            Properties.colorDown :
                            (
                                control.up.hovered ?
                                    Properties.colorHover :
                                    Properties.colorBrand
                            )
                ) :
                Properties.colorStrokeWeak

        color:
            control.enabled ?
                (
                    control.up.pressed ?
                        Properties.colorTextDown :
                        (
                            control.up.hovered ?
                                Properties.colorTextHover :
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
            text: Properties.plusIcon
            font.family: Fonts.fontAwesome.font.family
            font.pixelSize: Properties.fontSizeStepperIcon
            color: enabled ? parent.border.color : Properties.colorStrokeWeak
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    down.indicator: RoundedRectangle {
        width: Properties.textFieldHeight
        height: Properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Left
        radius: bg.radius
        border.width: bg.border.width
        border.color:
            control.enabled ?
                (
                    control.down.pressed ?
                        Properties.colorDown :
                            (
                                control.down.hovered ? Properties.colorHover :
                                                       Properties.colorBrand
                            )
                ) :
                Properties.colorStrokeWeak

        color:
            control.enabled ?
                (
                    control.down.pressed ?
                        Properties.colorTextDown :
                        (
                            control.down.hovered ? Properties.colorTextHover :
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
            text: Properties.minusIcon
            font.family: Fonts.fontAwesome.font.family
            font.pixelSize: Properties.fontSizeStepperIcon
            color: enabled ? parent.border.color : Properties.colorStrokeWeak
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    background: Rectangle {
        id: bg
        implicitWidth: Properties.textFieldHeight * 2 + control.inputFieldWidth
        border.width: Properties.borderWidth
        border.color: control.enabled ? Properties.colorBrand :
                                        Properties.colorStrokeWeak
        color: control.backgroundColor
        radius: Properties.radiusS

        // Apply dropshadow on stepper background when using lightTheme.
        layer.enabled: control.enabled && Properties.lightTheme
        layer.effect: ButtonElevationEffect {}
    }

    Item {
        // Unacceptable input message
        anchors {
            top: control.top
            topMargin: ( control.height - icon.height ) / 2
            left: control.right
            leftMargin: Properties.spacingM
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
            color: Properties.colorTransparent
            border.width: Properties.borderWidth
            border.color: Properties.colorError500

            Text {
                anchors.fill: parent

                text: Properties.invalidIcon
                font.family: Fonts.fontAwesome.font.family
                font.pixelSize: Properties.fontSizeUnacceptableInputIcon
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
                leftMargin: Properties.spacingXS
            }

            text: control.unacceptableInputMessage
            color: icon.border.color
        }
    }
}
