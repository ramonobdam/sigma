import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Universal

MenuBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    delegate: MenuBarItem {}

    contentItem: Row {
        spacing: control.spacing
        Repeater {
            model: control.contentModel
        }
    }

    background: Rectangle {
        implicitHeight: 28
        color: properties.colorTransparent
    }

    SigmaProperties {
        id: properties
    }

}
