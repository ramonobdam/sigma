import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Window

// Base window component with integrated titlebar on MacOS
Window {
    id: control

    property string windowTitle: ""
    property string settingsKey: ""
    property var settingsObject: null
    property bool persistentPosition: true
    property int titleBarHeightWindow:
        Math.min( SafeArea.margins.top, properties.minimumTitleBarHeight )
    property int topLabelSpacing: properties.spacingL
    property int bottomLabelSpacing: properties.spacingXS
    property int interLabelSpacing: properties.spacingS
    property int interFieldSpacing: properties.spacingM
    readonly property list<int> position: [ control.x, control.y ]

    function findScreenForPosition( position ) {
        // Return the screen for the given coordinates
        const screens = Qt.application.screens

        for ( let i = 0; i < screens.length; ++i ) {

            const px = position[ 0 ]
            const py = position[ 1 ]

            const s = screens[ i ]

            const sx = s.virtualX
            const sy = s.virtualY
            const sw = s.width
            const sh = s.height

            if (
                px >= sx &&
                px <  sx + sw &&
                py >= sy &&
                py <  sy + sh
            ) {
                return s
            }
        }

        return null
    }

    function centerOnPrimaryScreen() {
        // Center the window on the primary screen
        const screens = Qt.application.screens
        if ( screens.length === 0 ) {
            return
        }

        const s = screens[ 0 ]   // primary screen

        const sx = s.virtualX
        const sy = s.virtualY
        const sw = s.desktopAvailableWidth
        const sh = s.desktopAvailableHeight

        control.x = sx + ( sw - control.width ) / 2
        control.y = sy + ( sh - control.height ) / 2
    }

    function restorePosition() {
        // Restore window position if available, otherwise center on primary
        // screen
        if (
            !control.persistentPosition ||
            !control.settingsObject ||
            control.settingsKey === ""
        ) {
            return
        }

        let storedPostion = settingsObject.value(
            control.settingsKey,
            control.position
        )

        Qt.callLater( function() {
            let screen = control.findScreenForPosition( storedPostion )

            if ( screen ) {
                control.x = storedPostion[ 0 ]
                control.y = storedPostion[ 1 ]
            }
            else {
                control.centerOnPrimaryScreen()
            }
        } )
    }

    onVisibleChanged: {
        if ( control.visible ) {
            control.restorePosition()
        }
    }

    Component.onCompleted: {
            captionHelper.captionHeight = titleBarHeightWindow
    }

    Component.onDestruction: {
        // Store window position in persistent settings
        if (
            !control.persistentPosition ||
            !control.settingsObject ||
            control.settingsKey === ""
        ) {
            return
        }
        control.settingsObject.setValue( control.settingsKey, control.position )
    }

    x: transientParent.x + ( transientParent.width - control.width ) / 2
    y:  transientParent.y + ( transientParent.height - control.height ) / 2
    maximumWidth: width
    maximumHeight: height
    minimumWidth: width
    minimumHeight: height
    modality: Qt.ApplicationModal
    flags: properties.macOS ? (
                                  Qt.NoTitleBarBackgroundHint |
                                  Qt.ExpandedClientAreaHint |
                                  Qt.Dialog
                              ) :
                              Qt.Dialog

    title: properties.macOS ? "" : windowTitle
    color: properties.colorElevated

    SigmaProperties {
        id: properties
    }

    SigmaTitleBar {
        title: control.windowTitle
        titleBarHeight:
            Math.min( SafeArea.margins.top, properties.minimumTitleBarHeight)
    }
}
