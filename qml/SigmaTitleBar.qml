// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

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
    property int titleBarHeight: Properties.minimumTitleBarHeight
    property int titleBarLeftMargin: Properties.macOS ?
                                         Properties.titleBarTextLeftMargin :
                                         Properties.spacingM
    property int titleBarRightMargin: Properties.macOS ?
                                         Properties.titleBarTextLeftMargin :
                                         Properties.spacingM
    property string title: ""
    property int titleAlignment: Properties.macOS ? Text.AlignHCenter :
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

    visible: Properties.macOS || Properties.windows
    color: Properties.colorElevated

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
        color: Properties.macOS ? Properties.colorTextWeak :
                                  Properties.colorTextStrong
        font.family: Properties.macOS ? Fonts.interSemiBold.font.family :
                                        Fonts.inter.font.family
        font.pixelSize: Properties.fontSizeTitleBar
        font.bold: Properties.macOS
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

        visible: Properties.windows && control.showMinimizeButton
        hoverColor: Properties.colorTextHover
        onClicked: { Window.window.showMinimized() }

        Rectangle {
            anchors.centerIn: parent
            width: Properties.titleBarButtonIconWidth
            height: Properties.borderWidth

            color: Properties.colorTextStrong
            antialiasing: true
        }
    }

    TitleBarButton {
        id: maximizeButton

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: closeButton.left
        }

        property int offset: checked ? Properties.borderWidth : 0

        visible: Properties.windows && control.showMaximizeButton
        checkable: true
        checked: Window.window.visibility === Window.Maximized
        hoverColor: Properties.colorTextHover
        onClicked: { captionHelper.toggleMaximize( Window.window ) }

        // The icon of the maximize button is drawn using 2 rounded rectangles.
        // The filled area of the rectangles is covered by a rounded rectangle
        // with the same color to reduce the perceived line width after
        // anti-aliasing and bring it in line with the other buttons.
        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -maximizeButton.offset
            anchors.horizontalCenterOffset: maximizeButton.offset
            width: Properties.titleBarButtonIconWidth
            height: width

            color: parent.hovered ? parent.hoverColor : control.color
            border.width: Properties.borderWidth
            border.color: Properties.colorTextStrong
            radius: Properties.radiusXS

            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 2 * parent.border.width
                height: parent.height - 2 * parent.border.width
                color: parent.color
                radius: parent.radius - parent.border.width
            }
        }

        Rectangle {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: maximizeButton.offset
            anchors.horizontalCenterOffset: -maximizeButton.offset
            width: Properties.titleBarButtonIconWidth
            height: width

            color: parent.hovered ? parent.hoverColor : control.color
            border.width: Properties.borderWidth
            border.color: Properties.colorTextStrong
            radius: Properties.radiusXS

            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 2 * parent.border.width
                height: parent.height - 2 * parent.border.width
                color: parent.color
                radius: parent.radius - parent.border.width
            }
        }
    }

    TitleBarButton {
        id: closeButton

        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }

        visible: Properties.windows && control.showCloseButton
        text: Properties.titleBarCloseIcon
        hoverColor: Properties.colorClose
        font.family: Fonts.interSemiBold.font.family
        font.pixelSize: Properties.fontSizeTitleBarCloseIcon
        font.bold: true
        onClicked: { Window.window.close() }
    }

    DragHandler {
        enabled: Properties.macOS
        target: null
        onActiveChanged: {
            if ( active && control.Window.window ) {
                control.Window.window.startSystemMove()
            }
        }
    }

    MouseArea {
        anchors.fill: parent

        enabled: Properties.macOS
        onDoubleClicked: {
            if ( control.Window.window ) {
                captionHelper.toggleMaximize( Window.window )
            }
        }
    }
}
