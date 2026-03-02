import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// A custom titlebar is used on macOS that visually integrates with the window
Rectangle {
    id: control
    
    property bool showCloseButton: true
    property bool showMaximizeButton: true
    property bool showMinimizeButton: true
    property int titleBarHeight: properties.minimumTitleBarHeight
    property int titleBarLeftMargin: properties.macOS ?
                                         properties.titleBarTextLeftMargin :
                                         properties.spacingXS
    property int titleBarRightMargin: properties.macOS ?
                                         properties.titleBarTextLeftMargin :
                                         properties.spacingM
    property string title: ""
    property int titleAlignment: properties.macOS ? Text.AlignHCenter :
                                                    Text.AlignLeft
    property alias icon: icon
    
    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
    }
    height: visible ? titleBarHeight : 0

    visible: properties.macOS || properties.windows
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

        visible: properties.windows && control.showMinimizeButton
        hoverColor: properties.colorTextHover
        onClicked: { Window.window.showMinimized() }

        Rectangle {
            anchors.centerIn: parent
            width: properties.titleBarButtonIconWidth
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

        visible: properties.windows && control.showMaximizeButton
        checkable: true
        checked: Window.window.visibility === Window.Maximized
        hoverColor: properties.colorTextHover
        onClicked: { captionHelper.toggleMaximize( Window.window ) }

        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -maximizeButton.offset
            anchors.horizontalCenterOffset: maximizeButton.offset
            width: properties.titleBarButtonIconWidth
            height: width

            color: properties.colorTransparent
            border.width: properties.borderWidth
            border.color: properties.colorTextStrong
            radius: properties.radiusXS
        }

        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: maximizeButton.offset
            anchors.horizontalCenterOffset: -maximizeButton.offset
            width: properties.titleBarButtonIconWidth
            height: width

            color: parent.hovered ? parent.hoverColor : control.color
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

        visible: properties.windows && control.showCloseButton
        text: properties.titleBarCloseIcon
        hoverColor: properties.colorClose
        font.pixelSize: properties.fontSizeTitleBarCloseIcon
        onClicked: { Window.window.close() }
    }

    Image {
        id: icon

        anchors {
            left: parent.left
            leftMargin: properties.spacingXS
            verticalCenter: parent.verticalCenter
        }

        visible: properties.windows
        source: properties.appIcon
        width: visible ? properties.titleBarIconWidth : 0
        height: width
        fillMode: Image.PreserveAspectFit
        mipmap: true
    }
    
    SigmaText {
        id: title

        anchors {
            left: icon.right
            leftMargin: control.titleBarLeftMargin
            right: minimizeButton.left
            rightMargin: control.titleBarRightMargin
            top: parent.top
            bottom: parent.bottom
        }
        
        text: control.title
        color: properties.macOS ? properties.colorTextWeak :
                                  properties.colorTextStrong
        font.family: properties.macOS ? fonts.interSemiBold.font.family :
                                        fonts.inter.font.family
        font.pixelSize: properties.fontSizeTitleBar
        font.bold: properties.macOS
        clip: true
        elide: Text.ElideRight
        horizontalAlignment: control.titleAlignment
        verticalAlignment: Text.AlignVCenter
    }

    DragHandler {
        enabled: properties.windows
        target: null
        onActiveChanged: {
            if ( active && control.Window.window ) {
                control.Window.window.startSystemMove()
            }
        }
    }
}
