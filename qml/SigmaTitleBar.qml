import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import Sigma

// A custom titlebar is used on macOS that visually integrates with the window
Rectangle {
    id: control
    
    property bool showCloseButton: true
    property bool showMaximizeButton: true
    property bool showMinimizeButton: true
    property int titleBarHeight: properties.minimumTitleBarHeight
    property int titleBarLeftMargin: properties.macOS ?
                                         properties.titleBarTextLeftMargin :
                                         properties.spacingM
    property int titleBarRightMargin: properties.macOS ?
                                         properties.titleBarTextLeftMargin :
                                         properties.spacingM
    property string title: ""
    property int titleAlignment: properties.macOS ? Text.AlignHCenter :
                                                    Text.AlignLeft
    property list<Item> excludeItems: [
        minimizeButton,
        maximizeButton,
        closeButton
    ]

    function updateExclusions() {
        // Update items excluded from the hit detection
        if ( !Window.window || !captionHelper ) {
            return
        }

        let rects = []

        for ( let i = 0; i < excludeItems.length; ++i ) {
            let item = excludeItems[ i ]
            if ( !item || !item.visible ) {
                continue
            }

            let p = item.mapToItem( null, 0, 0 )
            rects.push( Qt.rect( p.x, p.y, item.width, item.height ) )
        }

        captionHelper.setExclusionRects( Window.window, rects )
    }

    function registerItem( item ) {
        if ( !item ) {
            return
        }

        excludeItems.push( item )

        item.xChanged.connect( updateExclusions )
        item.yChanged.connect( updateExclusions )
        item.widthChanged.connect( updateExclusions )
        item.heightChanged.connect( updateExclusions )
        item.visibleChanged.connect( updateExclusions )

        updateExclusions()
    }

    Component.onCompleted: { updateExclusions() }
    onWidthChanged: { updateExclusions() }
    onHeightChanged: { updateExclusions() }

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

    SigmaText {
        id: title

        anchors {
            left: parent.left
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

    DragHandler {
        enabled: properties.macOS
        target: null
        onActiveChanged: {
            if ( active && control.Window.window ) {
                control.Window.window.startSystemMove()
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        enabled: properties.macOS
        onDoubleClicked: {
            if ( control.Window.window ) {
                captionHelper.toggleMaximize( Window.window )
            }
        }
    }
}
