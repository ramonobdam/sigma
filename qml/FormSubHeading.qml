import QtQuick

// Sub heading used in forms with a text and sub text
Item {
    id: control

    property alias text: label.text
    property alias subText: subLabel.text

    anchors {
        left: parent.left
        right: parent.right
        topMargin: properties.spacingL
    }
    height: label.height +
            line.anchors.topMargin +
            line.height +
            subLabel.anchors.topMargin +
            subLabel.height

    SigmaProperties {
        id: properties
    }

    SigmaText {
        id: label

        anchors {
            left: parent.left
            right: parent.right
        }
        height: text.length > 0 ? contentHeight : 0

        color: properties.colorTextStrong
        elide: Text.ElideRight
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            top: label.bottom
            topMargin: control.text.length > 0 ? properties.spacingXS : 0
        }
        height: properties.borderWidth

        color: properties.colorStrokeWeak
    }

    SigmaText {
        id: subLabel

        anchors {
            left: parent.left
            right: parent.right
            top: line.bottom
            topMargin: height > 0 ? properties.spacingS : 0
        }
        height: text.length > 0 ? contentHeight : 0

        color: properties.colorTextWeak
        elide: Text.ElideRight
    }
}
