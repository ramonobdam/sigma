import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// Button component used in combination with an input field.
Button {
    id: button

    property string backgroundColor: properties.colorElevated
    property string buttonIcon: properties.symbolIcon
    property var control
    property bool open: checked

    height: properties.buttonHeight
    width: height
    padding: 0
    topInset: 0
    bottomInset: 0
    hoverEnabled: true
    checkable: true
    text: open ? properties.crossIcon : button.buttonIcon
    font.family: open ? fonts.fontAwesome.font.family :
                        fonts.interSemiBold.font.family
    font.pixelSize: open ? properties.fontSizeComboBoxIcon :
                           properties.fontSizeFieldButtonIcon
    font.bold: true

    contentItem: Text {
        id: text

        anchors.fill: parent

        text: button.text
        font: parent.font
        color: bg.border.color
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    background: RoundedRectangle {
        id: bg
        anchors.fill: parent
        height: properties.textFieldHeight
        cornerSide: RoundedRectangle.Direction.Left
        radius: properties.radiusS
        border.width: properties.borderWidth
        border.color:
            button.enabled ? ( button.pressed ?
                                  properties.colorDown :
                                  (
                                      button.hovered ? properties.colorHover :
                                                       properties.colorBrand
                                  )
                              ) :
                              properties.colorStrokeWeak

        color:
            button.enabled ? ( button.pressed ?
                                  properties.colorTextDown :
                                  (
                                      button.hovered ?
                                          properties.colorTextHover :
                                          button.backgroundColor
                                  )
                             ) :
                             button.backgroundColor

        // Apply dropshadow on background when using lightTheme.
        layer.enabled: button.enabled && properties.lightTheme
        layer.effect: ButtonElevationEffect {}

        Behavior on color {
            ColorAnimation {
                duration: properties.animationDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
