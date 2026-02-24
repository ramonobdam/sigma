import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// A custom titlebar is used on macOS that visually integrates with the window
Rectangle {
    id: titleBar
    
    property int titleBarHeight: properties.minimumTitleBarHeight
    property string title: ""
    
    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
    }
    height: properties.macOS ? titleBarHeight : 0
    visible: properties.macOS
    color: properties.colorTransparent

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
    
    SigmaText {
        anchors {
            left: parent.left
            leftMargin: properties.titleBarTextLeftMargin
            right: parent.right
            rightMargin: anchors.leftMargin
            top: parent.top
            bottom: parent.bottom
        }
        
        text: titleBar.title
        font.family: fonts.interSemiBold.font.family
        font.pixelSize: properties.fontSizeTitleBar
        font.bold: true
        clip: true
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    
    MouseArea {
        property var clickPos
        
        anchors.fill: parent
        
        // Move the window when the title bar is dragged.
        onPressed: ( mouse ) => {
                       clickPos  = Qt.point( mouse.x, mouse.y )
                   }
        onPositionChanged: ( mouse ) => {
                               var delta = Qt.point( mouse.x - clickPos.x,
                                                    mouse.y - clickPos.y
                                                    )
                               Window.window.x += delta.x
                               Window.window.y += delta.y
                           }
    }
}
