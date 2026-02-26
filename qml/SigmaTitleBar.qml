import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// A custom titlebar is used on macOS that visually integrates with the window
Rectangle {
    id: titleBar
    
    property bool showCloseButton: true
    property bool showMaximizeButton: true
    property bool showMinimizeButton: true
    property int titleBarHeight: properties.minimumTitleBarHeight
    property string title: ""
    
    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
    }
    height: titleBarHeight

    color: properties.colorElevated

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }

    TitleBarButton {
        id: minimizeButton

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: maximizeButton.left
        }

        visible: !properties.macOS && titleBar.showMinimizeButton
        hoverColor: properties.colorTextHover
        onClicked: { Window.window.showMinimized() }

        Rectangle {
            anchors.centerIn: parent
            width: properties.titleBarIconWidth
            height: properties.borderWidth

            color: properties.colorTextStrong
        }
    }

    TitleBarButton {
        id: maximizeButton

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: closeButton.left
        }

        property int offset: checked ? properties.borderWidth : 0

        visible: !properties.macOS && titleBar.showMaximizeButton
        checkable: true
        checked: Window.window.visibility === Window.Maximized
        hoverColor: properties.colorTextHover
        onToggled: {
            if ( Window.window.visibility === Window.Maximized ) {
                Window.window.showNormal()
            }
            else {
                Window.window.showMaximized()
            }
        }

        RoundedRectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -maximizeButton.offset
            anchors.horizontalCenterOffset: maximizeButton.offset
            width: properties.titleBarIconWidth
            height: width

            cornerSide: RoundedRectangle.Direction.All
            color: properties.colorTransparent
            border.width: properties.borderWidth
            border.color: properties.colorTextStrong
            radius: properties.radiusXS
        }

        RoundedRectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: maximizeButton.offset
            anchors.horizontalCenterOffset: -maximizeButton.offset
            width: properties.titleBarIconWidth
            height: width

            cornerSide: RoundedRectangle.Direction.All
            color: parent.hovered ? parent.hoverColor : titleBar.color
            border.width: properties.borderWidth
            border.color: properties.colorTextStrong
            radius: properties.radiusXS
        }
    }

    TitleBarButton {
        id: closeButton

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }

        visible: !properties.macOS && titleBar.showCloseButton
        text: properties.titleBarCloseIcon
        hoverColor: properties.colorClose
        font.pixelSize: properties.fontSizeTitleBarCloseIcon
        onClicked: { Window.window.close() }
    }
    
    SigmaText {
        anchors {
            left: parent.left
            leftMargin: properties.macOS ? properties.titleBarTextLeftMargin :
                                           properties.spacingS
            right: minimizeButton.left
            rightMargin: anchors.leftMargin
            top: parent.top
            bottom: parent.bottom
        }
        
        text: titleBar.title
        color: properties.macOS ? properties.colorTextWeak :
                                  properties.colorTextStrong
        font.family: fonts.interSemiBold.font.family
        font.pixelSize: properties.fontSizeTitleBar
        font.bold: properties.macOS
        clip: true
        elide: Text.ElideRight
        horizontalAlignment: properties.macOS ? Text.AlignHCenter :
                                                Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }
    
    DragHandler {
        target: null
        onActiveChanged: {
            if ( active && titleBar.Window.window ) {
                titleBar.Window.window.startSystemMove()
            }
        }
    }
}
