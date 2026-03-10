// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.impl
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Effects

ComboBox {
    id: control

    property bool dropShadow: true
    property bool valid: true
    property bool showUnacceptableInputMessage: true
    property bool useFieldButton: false
    property string backgroundColor: properties.colorElevated
    property string unacceptableInputMessage:
        properties.unacceptableInputValueText
    property int maximumLength: 20
    property int indicatorWidth: height
    property int maximumHeightPopup: 200
    property int animationDuration: properties.animationDuration
    property alias selectedText: textField.selectedText
    property alias cursorPosition: textField.cursorPosition

    function insert( position, text ) {
        textField.insert( position, text )
    }

    height: properties.textFieldHeight
    leftPadding: properties.spacingS
    rightPadding: properties.spacingS
    topPadding: properties.spacingXS
    bottomPadding: properties.spacingXS
    focus: true
    selectTextByMouse: editable
    textRole: "display"
    valueRole: "display"

    // Set active focus to this control when it is opened
    onDownChanged: {
        if ( control.down && !control.activeFocus ) {
            control.forceActiveFocus()
        }
    }

    // A return press on an active and closed combobox acts as accepted()
    Keys.onReturnPressed: {
        if ( !control.down ) {
            control.accepted()
        }
    }

    // Left/right keys only open/close the combobox when it is not editable
    Keys.onRightPressed: {
        if ( !control.editable ) {
            popup.open()
        }
    }
    Keys.onLeftPressed: {
        if (!control.editable ) {
            popup.close()
        }
    }
    Keys.priority: control.editable ? Keys.AfterItem : Keys.BeforeItem

    delegate: ItemDelegate {
        required property var model
        required property int index
        property bool current: control.currentIndex === index

        width: ListView.view.width
        height: control.height
        padding: 0
        leftPadding: control.leftPadding
        rightPadding: leftPadding
        highlighted: control.highlightedIndex === index

        contentItem: SigmaText {
            width: parent.width
            text: model[ control.textRole ]
            color: current ? properties.colorRaised : properties.colorTextStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            anchors.fill: parent
            color: current ? properties.colorBrand :
                             (
                                 highlighted ? properties.colorTextHover :
                                               control.backgroundColor
                              )

            Behavior on color {
                ColorAnimation {
                    duration: control.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    indicator: Item {
        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: control.indicatorWidth

        Text {
            id: icon

            anchors.fill: parent

            text: ( control.down && control.editable ) ?
                      properties.crossIcon :
                      properties.chevronDownIcon
            font.family: fonts.fontAwesome.font.family
            font.pixelSize: properties.fontSizeComboBoxIcon

            // When the ComboBox is not editable, the complete control is a
            // button.
            property bool indicatorHovered:
                control.editable ? hoverHandler.hovered : control.hovered

            color: enabled ? (
                                 indicatorHovered ? properties.colorHover :
                                                    properties.colorBrand
                             ) :
                             properties.colorStrokeWeak

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }

        HoverHandler {
            id: hoverHandler
        }
    }

    contentItem: SigmaTextField {
        id: textField

        anchors {
            fill: parent
            leftMargin: control.leftPadding
            rightMargin: indicator.width
            topMargin: control.topPadding
            bottomMargin: control.bottomPadding
        }
        leftPadding: 0
        rightPadding: 0
        topPadding: 0
        bottomPadding: 0

        focus: control.editable
        text: control.editable ? control.editText : control.displayText
        verticalAlignment: Qt.AlignVCenter
        maximumLength: control.maximumLength
        readOnly: !control.editable
        validator: control.validator
        autoScroll: control.editable
        showUnacceptableInputMessage: false
        showContextMenu: control.editable

        border.width: 0
        dropShadow: false
        backgroundItem.color: bg.color

        // When the ComboBox is not editable, open the popup when the TextField
        // is clicked.
        MouseArea {
            anchors.fill: parent
            enabled: !control.editable
            onClicked: { popup.open() }
            cursorShape: control.editable ? Qt.IBeamCursor : Qt.ArrowCursor
        }

        Keys.onReturnPressed: { control.accepted() }
        KeyNavigation.backtab: control.KeyNavigation.backtab
        KeyNavigation.tab: control.KeyNavigation.tab
    }

    background: RoundedRectangle {
        // Extra background that is used to 'visually connect' to the popup.
        id: bgFill

        anchors.fill: parent
        radius: bg.radius
        color: backgroundColor

        // Create 'visual connection' to the popup by removing the radius at the
        // bottom of the background and adding borders at the left and right
        // side (see the rectangles below).
        cornerSide: ( control.down && control.editable ) ? RoundedRectangle.Up :
                                                           RoundedRectangle.All

        // Apply dropshadow on background when using lightTheme.
        layer.enabled: control.dropShadow &&
                       control.enabled &&
                       properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Rectangle {
            anchors {
                left: parent.left
                top: parent.top
                topMargin: bgFill.radius
                bottom: parent.bottom
            }
            width: popupBg.border.width

            visible: control.down && control.editable
            color: popupBg.border.color
        }

        Rectangle {
            anchors {
                right: parent.right
                top: parent.top
                topMargin: bgFill.radius
                bottom: parent.bottom
            }
            width: popupBg.border.width

            visible: control.down && control.editable
            color: popupBg.border.color
        }

        RoundedRectangle {
            // Actual control background
            id: bg

            anchors.fill: parent

            radius: properties.radiusS
            color: (
                       control.activeFocus ||
                       ( control.hovered && control.enabled )
                   ) ? properties.colorTextHover :  control.backgroundColor

            border.width:
                ( !control.editable && control.down ) ? 0 :
                                                        properties.borderWidth
            border.color:
                control.enabled ?
                    (
                        ( !control.acceptableInput || !control.valid ) ?
                            properties.colorError500 :
                            (
                                control.activeFocus ?
                                    properties.colorBrand :
                                    properties.colorStrokeStrong
                            )
                    ) :
                    properties.colorStrokeWeak

            // Left side is square when a FieldButton is used and the left border is
            // hidden when the input is valid/acceptable.
            cornerSide: control.useFieldButton ?
                            RoundedRectangle.Direction.Right :
                            RoundedRectangle.Direction.All
            Rectangle {
                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: bg.border.width
                    bottom: parent.bottom
                    bottomMargin: bg.border.width
                }
                width: properties.borderWidth
                color: bg.color
                visible: control.useFieldButton &&
                         control.valid &&
                         control.acceptableInput
            }
        }
    }

    popup: Popup {
        id: popup

        property alias radius: popupBg.radius
        property alias border: popupBg.border

        y: control.editable ? control.height : 0
        width: control.width
        height: Math.min(
                    contentItem.implicitHeight + padding + topPadding,
                    control.maximumHeightPopup
                )

        padding: popupBg.border.width
        topPadding: control.editable ? properties.borderWidth :
                                       popupBg.border.width

        // Default transitions are disabled
        enter: Transition {
            // Override grow_fade_in
            NumberAnimation { property: "scale"; duration: 0 }
            NumberAnimation { property: "opacity"; duration: 0 }
        }
        exit: Transition {
            // Override shrink_fade_out
            NumberAnimation { property: "scale"; duration: 0 }
            NumberAnimation { property: "opacity"; duration: 0 }
        }

        contentItem: ListView {
            id: listView
            clip: true
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            boundsMovement: Flickable.StopAtBounds
            ScrollIndicator.vertical: SigmaScrollIndicator {}

            // Clip the rounded corners of the background by using a MultiEffect
            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: mask
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1.0
            }

            RoundedRectangle {
                id: mask
                width: listView.width
                height: listView.height
                cornerSide: popupBg.cornerSide
                radius: popupBg.radius - popupBg.border.width
                visible: false
                layer.enabled: true
                layer.smooth: true
            }
        }

        background: RoundedRectangle {
            id: popupBg

            cornerSide: control.editable ? RoundedRectangle.Down :
                                           RoundedRectangle.All
            radius: properties.radiusS
            color: properties.colorElevated
            border.width: properties.lightTheme ? 0 : properties.borderWidth
            border.color: properties.colorStrokeWeak
            layer.enabled: control.dropShadow &&
                           control.enabled &&
                           properties.lightTheme
            layer.effect: ButtonElevationEffect {}

            // 'Hide' top border when editable
            Rectangle {
                anchors {
                    left: parent.left
                    leftMargin: parent.border.width
                    right: parent.right
                    rightMargin: parent.border.width
                    top: parent.top
                }
                height: parent.border.width

                visible: control.editable

                color: parent.color
            }
        }
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
            topMargin: ( control.height - icon2.height ) / 2
            left: control.right
            leftMargin: properties.spacingM
        }
        visible: control.showUnacceptableInputMessage &&
                 ( !control.acceptableInput || !control.valid )

        Rectangle {
            id: icon2
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
                color: icon2.border.color
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        SigmaText {
            id: errorText
            anchors {
                top: icon2.top
                left: icon2.right
                leftMargin: properties.spacingXS
            }
            text: control.unacceptableInputMessage
            color: icon2.border.color
        }
    }
}
