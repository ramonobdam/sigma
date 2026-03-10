// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// Action item for native MenuBar on macOS and a styled MenuBar menu item on
// Windows
MenuItem {
    id: control

    property int fontSize: properties.fontSizeTitleBar
    property int animationDuration: properties.animationDuration
    property string backgroundColor: properties.colorElevated
    property alias shortcut: itemAction.shortcut

    implicitHeight: visible ? properties.rowHeight : 0
    implicitWidth: parent.implicitWidth
    padding: 0
    spacing: 0

    contentItem: properties.macOS ? null : content
    action: itemAction

    Action {
        id: itemAction

        text: control.text
        enabled: control.enabled
        checkable: control.checkable
        checked: control.checked
        icon: control.icon
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
            width: properties.menuItemMargins

            font.pixelSize: control.fontSize
            font.family: fonts.fontAwesome.font.family
            text: control.checked ? properties.validIcon : ""
            color: control.enabled ? (
                                       control.down ? properties.colorRaised :
                                                      properties.colorTextStrong
                                     ) :
                                     properties.colorStrokeStrong
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
                                       control.down ? properties.colorRaised :
                                                      properties.colorTextStrong
                                     ) :
                                     properties.colorStrokeStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        SigmaText {
            id: shortcut

            anchors {
                right: parent.right
                rightMargin: properties.menuItemMargins
                top: parent.top
                bottom: parent.bottom
            }
            width: properties.menuItemShortcutWidth

            font.pixelSize: control.fontSize
            text: shortcutItem.nativeText
            color: text.color
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight

            Shortcut{
                // Small helper to enable nativeText
                id: shortcutItem

                sequence: control.shortcut
            }
        }
    }

    background: Rectangle {
        anchors.fill: parent
        color: control.down ? properties.colorBrand :
                              (
                                  control.highlighted ?
                                      properties.colorTextHover :
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

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
