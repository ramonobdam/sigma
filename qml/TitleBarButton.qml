// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Sigma

// Button component used for title bar buttons
Button {
    id: control

    property string hoverColor: Properties.colorClose
    property alias textItem: textItem
    
    width: visible ? Properties.titeBarButtonWidth : 0
    spacing: 0
    topInset: 0
    bottomInset: 0
    hoverEnabled: true
    enabled: visible
    
    contentItem: Text {
        id: textItem
        anchors.fill: parent
        
        text: control.text
        color: control.hovered ?
                   (
                       Properties.lightTheme ? Properties.colorBase :
                                               Properties.colorTextStrong
                   ) :
                   Properties.colorTextStrong
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        antialiasing: true
    }
    
    background: Rectangle {
        color: control.hovered ? control.hoverColor :
                                 Properties.colorTransparent
    }
}
