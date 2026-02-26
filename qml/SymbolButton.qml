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

    MouseArea {
        // A MouseArea is used so the 'click' on the button can be accepted
        // and the 'press' of the click doesn't trigger
        // 'CloseOnReleaseOutsideParent' on the popup. Otherwise the popup
        // will close on 'release' on the button.
        anchors.fill: parent

        onClicked: ( event ) => {
            event.accepted = true
            button.clicked()
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaToolTip {
        id: control

        visible: button.hovered && button.toolTipText.length > 0
        text: button.enabled ? button.toolTipText : button.toolTipTextDisabled
    }
}
