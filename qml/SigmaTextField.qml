// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Styled TextField component
TextField {
    id: control

    property int radius: properties.radiusS
    property bool dropShadow: true
    property string backgroundColor: properties.colorElevated
    property bool valid: true
    property bool showUnacceptableInputMessage: true
    property bool useFieldButton: false
    property bool textSelected: selectedText.length > 0
    property bool showContextMenu: true
    property string unacceptableInputMessage:
        properties.unacceptableInputValueText
    property alias border: bg.border
    property alias backgroundItem: bg

    height: properties.textFieldHeight
    leftPadding: properties.spacingS
    rightPadding: properties.spacingS
    topPadding: properties.spacingXS
    bottomPadding: properties.spacingXS

    font.family: fonts.inter.font.family
    font.pixelSize: properties.fontSizeBody
    focus: true
    color: enabled ? properties.colorTextStrong : properties.colorStrokeWeak
    selectionColor: properties.colorBrand
    selectByMouse: !readOnly
    selectedTextColor: backgroundColor
    placeholderTextColor: enabled ? properties.colorStrokeStrong :
                                    properties.colorStrokeWeak

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
                   properties.colorTextHover :
                   backgroundColor
        border.color:
            control.enabled ?
                ( ( !control.acceptableInput || !control.valid ) ?
                        properties.colorError500 :
                        (
                            control.activeFocus ?
                                properties.colorBrand :
                                properties.colorStrokeStrong
                        )
                ) :
                properties.colorStrokeWeak
        border.width: properties.borderWidth
        radius: control.radius

        // Apply dropshadow on background when using lightTheme.
        layer.enabled: control.dropShadow &&
                       control.enabled &&
                       properties.lightTheme
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
            width: properties.borderWidth

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
            leftMargin: properties.spacingM
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

    Component {
        id: contextMenu

        BaseContextMenu {
            width: properties.contextMenuWidthTextField

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
                height: properties.borderWidth
                color: properties.colorStrokeWeak
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
                height: properties.borderWidth
                color: properties.colorStrokeWeak
            }

            ContextMenuItem {
                enabled: control.text.length > 0
                text: "Select All"
                onTriggered: control.selectAll()
            }
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
