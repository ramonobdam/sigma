// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick

// This is a specialized version of the stepper for multiples of 10 shown in
// scientific notation. The stepper 'value' is the exponent while number
// contains '10 ^ value'. The TextField is not editable.
Stepper {
    id: control

    property int decimals: 0
    property int number: valueToNumber( value )
    property int numberFrom
    property int numberTo
    property string fromString: textFromValue ( from, locale )
    property string toString: textFromValue ( to, locale )

    function numberToValue( decimal ) {
        return Math.floor ( Math.log10 ( decimal ) )
    }

    function valueToNumber ( integer ) {
        return Math.pow ( 10, integer )
    }

    inputFieldWidth: 65
    editable: false
    from: numberToValue( numberFrom )
    to: numberToValue( numberTo )

    validator: DoubleValidator {
        bottom: control.numberFrom
        top: control.numberTo
        decimals: control.decimals
        notation: DoubleValidator.ScientificNotation
    }

    textFromValue: function( value, locale ) {
        return Number( valueToNumber( value ) ).toLocaleString(
                                                    locale,
                                                    'e',
                                                    control.decimals
                                                )
    }
    valueFromText: function( text, locale ) {
        return numberToValue( Number.fromLocaleString( locale, text ) )
    }
}
