import QtQuick

// Styled MenuBar menu item based on ContextMenuItem
ContextMenuItem {
    id: control

    property int fontSize: properties.fontSizeTitleBar

    contentItem: Item {
        SigmaText {
            id: icon

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }
            width: height

            font.pixelSize: control.fontSize
            font.family: fonts.fontAwesome.font.family
            text: control.checked ? properties.validIcon : ""
            color: control.enabled ? (
                                         control.down ? properties.colorRaised :
                                                        properties.colorTextStrong
                                      ) :
                                     properties.colorStrokeStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        SigmaText {
            id: text

            anchors {
                left: icon.right
                right: shortcut.left
                top: parent.top
                bottom: parent.bottom
            }

            font.pixelSize: control.fontSize
            text: control.text
            color: control.enabled ? (
                                         control.down ? properties.colorRaised :
                                                        properties.colorTextStrong
                                      ) :
                                     properties.colorStrokeStrong
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        SigmaText {
            id: shortcut

            anchors {
                right: parent.right
                top: parent.top
                bottom: parent.bottom
            }
            width: properties.menuItemShortcutWidth

            font.pixelSize: control.fontSize
            text: control.action.shortcut ? control.action.shortcut : ""
            color: text.color
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignRight
        }
    }

    SigmaFonts {
        id: fonts
    }
}
