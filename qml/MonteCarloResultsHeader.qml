import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Component used for the Monte Carlo results table left column (i.e. header)
Item {
    id: control

    property string text

    anchors {
        left: parent.left
        right: parent.right
        rightMargin: -( properties.spacingM ) / 2
    }
    height: properties.rowHeight + line.height

    SigmaProperties {
        id: properties
    }

    Rectangle {
        id: separatorLeft

        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            topMargin: ( properties.rowHeight - properties.separatorHeight ) / 2
            bottomMargin:
                ( properties.rowHeight - properties.separatorHeight ) / 2 +
                line.height
        }
        width: properties.borderWidth

        color: properties.colorStrokeWeak
    }

    Rectangle {
        id: separatorRight

        anchors {
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            topMargin: ( properties.rowHeight - properties.separatorHeight ) / 2
            bottomMargin:
                ( properties.rowHeight - properties.separatorHeight ) / 2 +
                line.height

        }
        width: properties.borderWidth

        color: properties.colorStrokeWeak
    }

    SigmaText {
        anchors {
            left: parent.left
            right: separatorRight.left
            top: parent.top
            bottom: line.top
        }

        text: control.text
        color: properties.colorTextStrong
        padding: properties.spacingS
        elide: Text.ElideRight
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        height: properties.borderWidth

        color: properties.colorStrokeWeak
    }
}
