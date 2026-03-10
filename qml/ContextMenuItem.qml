// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// Styled menu item component for context menus
MenuItem {
    id: control

    property int animationDuration: properties.animationDuration
    property string backgroundColor: properties.colorElevated
    property alias properties: properties
    property alias shortcut: itemAction.shortcut

    height: visible ? properties.rowHeight : 0
    padding: 0
    spacing: 0

    action: Action {
        id: itemAction
    }

    contentItem: SigmaText {
        id: text

        width: control.width
        text: control.text
        color: control.enabled ? (
                                     control.down ? properties.colorRaised :
                                                    properties.colorTextStrong
                                  ) :
                                 properties.colorStrokeStrong
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
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
}
