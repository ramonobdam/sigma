import QtQuick
import QtQuick.Controls

// Base component that can be combined with an input field to add a button and
// popup to insert symbols or operators
FocusScope {
    id: control

    property string symbols: "αβγδεζηθιξλμνξπρστυφχψωΓΔΘΛΞΠΣΦΨΩ"
    property var field
    property var buttonList
    property int rows: Math.ceil( buttonList.count / columns )
    property int columns: 11
    property alias fieldButton: fieldButton
    property alias symbolPopup: popup
    property alias buttonIcon: fieldButton.buttonIcon
    property alias buttonFont: fieldButton.font
    property alias properties: properties

    function insertAtCursor( text ) {
        // Insert text at the cursor position or replace the
        // selection of the field
        if ( text.length > 0 )
        {
            let pos = field.cursorPosition
            if ( field.selectedText.length > 0 ) {
                // delete the selected characters
                let selectionStart = field.selectionStart
                let selectionEnd = field.selectionEnd
                field.remove( selectionStart, selectionEnd )
                pos = selectionStart
            }
            field.insert( pos, text )
            // Locate the cursor inside the new brackets if
            // present
            let adjust = text.endsWith( "()" ) ? -1 : 0
            field.cursorPosition = pos + text.length + adjust
            field.forceActiveFocus()
        }
    }

    width: properties.fieldWidth
    height: childrenRect.height
    focus: true

    // ListModel containing the text and symbol to be inserted for each button
    // of the menu
    buttonList: ListModel {
        Component.onCompleted: {
            for ( let i = 0; i < symbols.length; ++i ) {
                append( { "buttonText": symbols[ i ],
                          "symbol": symbols[ i ],} )
            }
        }
    }

    FieldButton {
        id: fieldButton

        focus: false
        onToggled: popup.visible = !popup.visible
        open: popup.visible
    }

    Popup {
        id: popup

        x: 0
        y: parent.height
        padding: bg.border.width
        width: ( properties.buttonHeight + flowContainer.spacing ) * columns +
               flowContainer.spacing +
               padding * 2
        height: ( properties.buttonHeight + flowContainer.spacing ) * rows +
                flowContainer.spacing +
                padding * 2
        popupType: Popup.Item
        closePolicy: Popup.CloseOnPressOutsideParent

        onVisibleChanged: visible ? field.forceActiveFocus() : {}

        background: Rectangle {
            id: bg

            radius: properties.radiusS
            color: properties.colorElevated
            border.width: properties.lightTheme ? 0 : properties.borderWidth
            border.color: properties.colorStrokeWeak
            layer.enabled: properties.lightTheme
            layer.effect: ButtonElevationEffect {}
        }

        Flow {
            id: flowContainer

            anchors.fill: parent
            padding: properties.spacingXXS
            spacing: padding

            Instantiator {
                model: buttonList
                onObjectAdded: (index, object) => object.parent = flowContainer

                delegate: SymbolButton {
                    text: buttonText
                    onClicked: insertAtCursor( symbol )
                }
            }
        }
    }

    SigmaProperties {
        id: properties
    }
}
