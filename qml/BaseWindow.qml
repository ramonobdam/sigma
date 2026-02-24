import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Base window component with integrated titlebar on MacOS
Window {
    id: control

    property string windowTitle: ""
    property int titleBarHeightWindow:
        Math.min( SafeArea.margins.top, properties.minimumTitleBarHeight )
    property int topLabelSpacing: properties.spacingL
    property int bottomLabelSpacing: properties.spacingXS
    property int interLabelSpacing: properties.spacingS
    property int interFieldSpacing: properties.spacingM

    // Fix to prevent the stored window vertical position (i.e. y) from shifting
    // up with the 'titleBarHeight' when using 'Qt.ExpandedClientAreaHint'
    Component.onDestruction: {
        if ( properties.macOS ) {
            y = y + titleBarHeightWindow
        }
    }

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
