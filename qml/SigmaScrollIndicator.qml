import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// ScrollIndicator component with custom styling
ScrollIndicator {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    padding: 2

    contentItem: Rectangle {
        implicitWidth: 4
        implicitHeight: 4
        radius: implicitWidth / 2
        opacity: 0.7
        color: properties.colorStrokeStrong
        visible: control.size < 1.0
    }

    SigmaProperties {
        id: properties
    }
}
