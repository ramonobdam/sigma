// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import Sigma

// Action item for native MenuBar on macOS and a styled MenuBar menu item on
// Windows
MenuItem {
    id: control

    property int fontSize: Properties.fontSizeTitleBar
    property int animationDuration: Properties.animationDuration
    property string backgroundColor: Properties.colorElevated
    property var shortcut: undefined

    implicitHeight: visible ? Properties.rowHeight : 0
    implicitWidth: parent.implicitWidth
    padding: 0
    spacing: 0

    contentItem: Properties.macOS ? null : content
    action: itemAction

    Action {
        id: itemAction

        text: control.text
        enabled: control.enabled
        checkable: control.checkable
        checked: control.checked
        icon: control.icon
        shortcut: control.shortcut
        onToggled: { control.toggled() }
    }

    Item {
        id: content

        SigmaText {
            id: icon

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: Properties.menuItemMargins

            font.pixelSize: control.fontSize
            font.family: Fonts.fontAwesome.font.family
            text: control.checked ? Properties.validIcon : ""
            color: control.enabled ? (
                                       control.down ? Properties.colorRaised :
                                                      Properties.colorTextStrong
                                     ) :
                                     Properties.colorStrokeStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        SigmaText {
            id: text

            anchors {
                left: icon.right
                right: shortcut.left
                top: parent.top
                bottom: parent.bottom
            }

            font.pixelSize: control.fontSize
            text: control.text
            color: control.enabled ? (
                                       control.down ? Properties.colorRaised :
                                                      Properties.colorTextStrong
                                     ) :
                                     Properties.colorStrokeStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        SigmaText {
            id: shortcut

            anchors {
                right: parent.right
                rightMargin: Properties.menuItemMargins
                top: parent.top
                bottom: parent.bottom
            }
            width: Properties.menuItemShortcutWidth

            font.pixelSize: control.fontSize
            text: shortcutItem.nativeText
            color: text.color
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight

            Shortcut{
                // Small helper to enable nativeText. The actual shortcut is
                // disabled.
                id: shortcutItem

                enabled: false
                sequences:
                    control.shortcut !== undefined ? [ control.shortcut ] : []
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color: control.down ? Properties.colorBrand :
                              (
                                  control.highlighted ?
                                      Properties.colorTextHover :
                                      control.backgroundColor
                              )

        Behavior on color {
            ColorAnimation {
                duration: control.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    indicator: Item {}

    arrow: Item {}
}
