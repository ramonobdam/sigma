// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// ComboBox combined with a SymBolField results in a ComboBox with a button to
// insert symbols via a popup
SymbolField {
    id: control

    property alias maximumLength: comboBox.maximumLength
    property alias unacceptableInputMessage: comboBox.unacceptableInputMessage
    property alias valid: comboBox.valid
    property alias comboBox: comboBox
    property alias model: comboBox.model
    property alias editable: comboBox.editable
    property alias editText: comboBox.editText
    property alias selectedText: comboBox.selectedText
    property alias currentIndex: comboBox.currentIndex
    property alias cursorPosition: comboBox.cursorPosition
    property alias maximumHeightPopup: comboBox.maximumHeightPopup

    function find( text, flags = Qt.MatchExactly ) {
        return comboBox.find( text, flags )
    }

    function indexOfValue( value ) {
        return comboBox.indexOfValue( value )
    }

    function insert ( position, text ) {
        comboBox.insert( position, text )
    }

    field: comboBox
    symbols: "⁰¹²³⁴⁵⁶⁷⁸⁹⁺⁻₀₁₂₃₄₅₆₇₈₉₊₋αβγδεζηθιξϰλμνξπρστυφχψωΓΔΘΛΞΠΣΦΨΩ°·"
    columns: 12

    ComboBoxListModel {
        id: comboBox

        anchors {
            left: fieldButton.right
        }
        width: control.width - fieldButton.width

        useFieldButton: true
        onActiveFocusChanged: !activeFocus ? symbolPopup.close() : {}
        onDownChanged: down ? symbolPopup.close() : {}

        Keys.onPressed: ( event ) => {
            if ( event.key === Qt.Key_Escape ) {
                if ( symbolPopup.visible ) {
                    symbolPopup.close()
                    event.accepted = true
                    return
                }
                event.accepted = false
            }
        }
    }
}
