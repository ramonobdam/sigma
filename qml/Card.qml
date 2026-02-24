import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// Styled card component with rounded corners, background, border and heading
Pane {
    id: control

    property string title
    property alias header: header

    anchors {
        fill: parent
    }
    padding: properties.spacingM
    background: Rectangle {
        color: properties.colorRaised
        radius: properties.radiusM
        border.width: properties.lightTheme ? 0 : properties.borderWidth
        border.color: properties.colorStrokeWeak
        layer.enabled: properties.lightTheme
        layer.effect: RoundedElevationEffect {
            elevation: properties.elevationRaised
            roundedScale: Material.ExtraSmallScale
        }
    }

    SigmaProperties {
        id: properties
    }

    Item {
        id: header

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: heading.height + properties.spacingM

        Heading {
            id: heading
            text: control.title
        }
    }
}
