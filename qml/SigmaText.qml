// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick

// Default styled text component
Text {
    font.family: fonts.inter.font.family
    font.pixelSize: properties.fontSizeBody
    //width: contentWidth
    height: contentHeight
    color: properties.colorTextWeak

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
