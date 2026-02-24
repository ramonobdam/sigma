import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

// Button component used for the popups to insert special (greek) characters or
// mathematical operators
Button {
    id: button

    property string toolTipText
    property string toolTipTextDisabled: toolTipText
    property string backgroundColor: properties.colorElevated
    property int scale: Math.ceil( text.contentWidth / height )
    property alias textColor: text.color

    height: properties.buttonHeight
    width: scale * height + ( scale - 1) * properties.spacingXXS
    focus: true
    hoverEnabled: true

    KeyNavigation.left: KeyNavigation.backtab
    KeyNavigation.right: KeyNavigation.tab
    Keys.onReturnPressed: clicked()

    contentItem: SigmaText {
        id: text

        anchors.fill: parent

        text: button.text
        color: button.down ? button.backgroundColor : properties.colorTextStrong
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        anchors.fill: parent

        color: button.down ? (
                                 properties.lightTheme ?
                                     properties.colorTextWeak :
                                     properties.colorStrokeStrong
                             ) :
                             button.hovered ? properties.colorTextDown :
                                              properties.colorTextHover
        radius: properties.radiusS - properties.borderWidth

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

    ToolTip {
        id: control
        delay: properties.toolTipDelay
        timeout: properties.toolTipTimeout
        visible: button.hovered && button.toolTipText.length > 0
        padding: properties.spacingXS

        contentItem: SigmaText {
            text: button.enabled ? button.toolTipText :
                                   button.toolTipTextDisabled
            color: properties.colorTextStrong
            textFormat: Text.StyledText
        }

        background: Rectangle {
            color: properties.colorFill
            border.color: properties.colorStrokeWeak
            radius: properties.radiusS
            layer.enabled: true
            layer.effect: RoundedElevationEffect {
                elevation: properties.elevationElevated
                roundedScale: Material.ExtraSmallScale
            }
        }
    }
}
