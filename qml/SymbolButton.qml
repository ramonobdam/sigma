// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// Button component used for the popups to insert special (greek) characters or
// mathematical operators
Button {
    id: button

    property string toolTipText
    property string toolTipTextDisabled: toolTipText
    property string backgroundColor: Properties.colorElevated
    property int scale: Math.ceil( text.contentWidth / height )
    property alias textColor: text.color

    height: Properties.buttonHeight
    width: scale * height + ( scale - 1) * Properties.spacingXXS
    focus: true
    hoverEnabled: true

    KeyNavigation.left: KeyNavigation.backtab
    KeyNavigation.right: KeyNavigation.tab
    Keys.onReturnPressed: clicked()

    contentItem: SigmaText {
        id: text

        anchors.fill: parent

        text: button.text
        color: button.down ? button.backgroundColor : Properties.colorTextStrong
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        anchors.fill: parent

        color: mouseArea.pressed ? (
                                        Properties.lightTheme ?
                                            Properties.colorTextWeak :
                                            Properties.colorStrokeStrong
                                    ) :
                                    button.hovered ? Properties.colorTextDown :
                                                     Properties.colorTextHover
        radius: Properties.radiusS - Properties.borderWidth

        Behavior on color {
            ColorAnimation {
                duration: Properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    MouseArea {
        id: mouseArea

        // A MouseArea is used so the 'click' on the button can be accepted
        // and the 'press' of the click doesn't trigger
        // 'CloseOnReleaseOutsideParent' on the popup. Otherwise the popup
        // will close on 'release' on the button.
        anchors.fill: parent

        onClicked: ( event ) => {
            event.accepted = true
            button.clicked()
        }
    }

    SigmaToolTip {
        id: control

        visible: button.hovered && button.toolTipText.length > 0
        text: button.enabled ? button.toolTipText : button.toolTipTextDisabled
    }
}
