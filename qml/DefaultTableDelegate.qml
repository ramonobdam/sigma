import QtQuick

// Table delegate with optional valid icon on the left and color scale on the
// right
TableDelegate {
    id: control

    property bool useScale: false
    property bool useValid: false
    property string icon: decoration ? properties.validIcon :
                                       properties.invalidIcon
    property string validColor: decoration ? properties.colorSuccess500 :
                                             properties.colorError500
    required property int decoration

    Rectangle {
        id: validRectangle

        anchors {
            left: parent.left
            top: parent.top
            bottom: line.top
        }
        width: control.useValid ? properties.validIconWidth : 0
        visible: control.useValid
        color: control.validColor

        Text {
            anchors.fill: parent

            text: control.icon
            font.family: fonts.fontAwesome.font.family
            font.pixelSize: properties.fontSizeValidIcon
            color: control.backgroundColor
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }
    }

    Rectangle {
        id: separatorLeft

        anchors {
            left: validRectangle.right
            top: parent.top
            bottom: line.top
        }
        width: control.useValid ? properties.borderWidth : 0

        visible: control.useValid
        color: control.backgroundColor
        antialiasing: true
    }

    Rectangle {
        id: textBox

        anchors {
            left: separatorLeft.right
            right: separatorRight.left
            top: parent.top
            bottom: line.top
            margins: properties.spacingXXS
        }

        color: properties.colorTransparent
        radius: properties.radiusS
        border.width: control.hasFocus ? properties.borderWidth : 0
        border.color: control.backgroundColor

        SigmaText {
            anchors.fill: parent

            text: control.display
            color: control.currentRow ? control.backgroundColor :
                                        properties.colorTextWeak
            textFormat: Text.PlainText
            elide: Text.ElideRight
            padding: properties.spacingS - textBox.anchors.margins
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignLeft
        }
    }

    Rectangle {
        id: separatorRight

        anchors {
            left: scaleRectangle.left
            top: parent.top
            bottom: line.top
        }
        width: control.useScale ? properties.borderWidth : 0

        visible: control.useScale
        color: control.backgroundColor
        antialiasing: true
    }

    Rectangle {
        id: scaleRectangle

        anchors {
            right: parent.right
            top: parent.top
            bottom: line.top
        }
        width: control.useScale ? properties.colorScaleWidth : 0

        visible: control.useScale
        color: (
                    control.useScale &&
                    control.display != "-" &&
                    control.decoration >= 0 &&
                    control.decoration < properties.colorScale.length
               ) ?
                    properties.colorScale[ control.decoration ] :
                    properties.colorTransparent
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
        antialiasing: true
    }
}
