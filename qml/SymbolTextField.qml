// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// TextField combined with a SymBolField results in a TextField with a button to
// insert symbols via a popup
SymbolField {
    id: control

    property alias maximumLength: textField.maximumLength
    property alias unacceptableInputMessage: textField.unacceptableInputMessage
    property alias text: textField.text
    property alias valid: textField.valid
    property alias textField: textField

    field: textField

    SigmaTextField {
        id: textField

        anchors {
            left: fieldButton.right
        }
        width: parent.width - fieldButton.width

        useFieldButton: true
        onActiveFocusChanged: !activeFocus ? symbolPopup.close() : {}

        KeyNavigation.tab: control.KeyNavigation.tab
        KeyNavigation.backtab: control.KeyNavigation.backtab
        KeyNavigation.up: control.KeyNavigation.up
        KeyNavigation.down: control.KeyNavigation.down

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
