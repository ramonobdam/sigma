// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Sigma

// Styled TextField component
TextField {
    id: control

    property int radius: Properties.radiusS
    property bool dropShadow: true
    property string backgroundColor: Properties.colorElevated
    property bool valid: true
    property bool showUnacceptableInputMessage: true
    property bool useFieldButton: false
    property bool textSelected: selectedText.length > 0
    property bool showContextMenu: true
    property string unacceptableInputMessage:
        Properties.unacceptableInputValueText
    property alias border: bg.border
    property alias backgroundItem: bg

    height: Properties.textFieldHeight
    leftPadding: Properties.spacingS
    rightPadding: Properties.spacingS
    topPadding: Properties.spacingXS
    bottomPadding: Properties.spacingXS

    font.family: Fonts.inter.font.family
    font.pixelSize: Properties.fontSizeBody
    focus: true
    color: enabled ? Properties.colorTextStrong : Properties.colorStrokeWeak
    selectionColor: Properties.colorBrand
    selectByMouse: !readOnly
    selectedTextColor: backgroundColor
    placeholderTextColor: enabled ? Properties.colorStrokeStrong :
                                    Properties.colorStrokeWeak

    ContextMenu.onRequested: position => {
        if ( showContextMenu ) {
            const menu = contextMenu.createObject( control )
            menu.popup( position )
        }
    }

    background: RoundedRectangle {
        id: bg

        anchors.fill: parent

        color: ( control.activeFocus || control.hovered && control.enabled ) ?
                   Properties.colorTextHover :
                   backgroundColor
        border.color:
            control.enabled ?
                ( ( !control.acceptableInput || !control.valid ) ?
                        Properties.colorError500 :
                        (
                            control.activeFocus ?
                                Properties.colorBrand :
                                Properties.colorStrokeStrong
                        )
                ) :
                Properties.colorStrokeWeak
        border.width: Properties.borderWidth
        radius: control.radius

        // Apply dropshadow on background when using lightTheme.
        layer.enabled: control.dropShadow &&
                       control.enabled &&
                       Properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        // Left side is square when a FieldButton is used and the left border is
        // hidden when the input is valid/acceptable.
        cornerSide: control.useFieldButton ? RoundedRectangle.Direction.Right :
                                             RoundedRectangle.Direction.All
        Rectangle {
            anchors {
                left: parent.left
                top: parent.top
                topMargin: bg.border.width
                bottom: parent.bottom
                bottomMargin: bg.border.width
            }
            width: Properties.borderWidth

            color: bg.color
            visible: control.useFieldButton &&
                     control.valid &&
                     control.acceptableInput
        }
    }

    cursorDelegate: Rectangle {
        id: cursor

        color: control.color
        width: 1
        visible: control.activeFocus &&
                 !control.readOnly &&
                 control.selectionStart === control.selectionEnd

        Connections {
            target: control
            function onCursorPositionChanged() {
                // keep a moving cursor visible
                cursor.opacity = 1
                timer.restart()
            }
        }

        Timer {
            id: timer
            running: control.activeFocus &&
                     !control.readOnly &&
                     interval !== 0
            repeat: true
            interval: Application.styleHints.cursorFlashTime / 2
            onTriggered: cursor.opacity = !cursor.opacity ? 1 : 0
            // force the cursor visible when gaining focus
            onRunningChanged: cursor.opacity = 1
        }
    }

    HoverHandler {
        enabled: control.readOnly
        cursorShape: Qt.ArrowCursor
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
                 ( !control.acceptableInput || !control.valid )

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

    Component {
        id: contextMenu

        BaseContextMenu {
            width: Properties.contextMenuWidthTextField

            ContextMenuItem {
                enabled: control.canUndo
                text: "Undo"
                onTriggered: control.undo()
            }

            ContextMenuItem {
                enabled: control.canRedo
                text: "Redo"
                onTriggered: control.redo()
            }

            Rectangle {
                height: Properties.borderWidth
                color: Properties.colorStrokeWeak
            }

            ContextMenuItem {
                enabled: control.textSelected
                text: "Cut"
                onTriggered: control.cut()
            }

            ContextMenuItem {
                enabled: control.textSelected
                text: "Copy"
                onTriggered: control.copy()
            }

            ContextMenuItem {
                enabled: control.canPaste
                text: "Paste"
                onTriggered: control.paste()
            }

            ContextMenuItem {
                enabled: control.textSelected
                text: "Delete"
                onTriggered: {
                    // delete the selected characters
                    control.remove(
                                control.selectionStart,
                                control.selectionEnd
                    )
                }
            }

            Rectangle {
                height: Properties.borderWidth
                color: Properties.colorStrokeWeak
            }

            ContextMenuItem {
                enabled: control.text.length > 0
                text: "Select All"
                onTriggered: control.selectAll()
            }
        }
    }
}
