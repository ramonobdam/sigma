import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Button component used for title bar buttons
Button {
    id: control

    property string hoverColor: properties.colorClose
    property alias textItem: textItem
    
    width: visible ? properties.titeBarButtonWidth : 0
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
                       properties.lightTheme ? properties.colorBase :
                                               properties.colorTextStrong
                       ) :
                   properties.colorTextStrong
        font: control.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    background: Rectangle {
        color: control.hovered ? control.hoverColor :
                                 properties.colorTransparent
        
        Behavior on color {
            ColorAnimation {
                duration: properties.animationDuration
                easing.type: Easing.InOutQuad
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
