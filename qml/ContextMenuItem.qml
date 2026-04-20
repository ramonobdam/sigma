// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import Sigma

// Styled menu item component for context menus
MenuItem {
    id: control

    property int animationDuration: Properties.animationDuration
    property string backgroundColor: Properties.colorElevated
    property alias shortcut: itemAction.shortcut

    height: visible ? Properties.rowHeight : 0
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
                                     control.down ? Properties.colorRaised :
                                                    Properties.colorTextStrong
                                  ) :
                                 Properties.colorStrokeStrong
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
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
