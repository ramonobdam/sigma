// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick

// Text heading component
Text {
    height: contentHeight
    font.family: fonts.interSemiBold.font.family
    font.pixelSize: properties.fontSizeHeader
    font.bold: true
    font.letterSpacing: properties.headingLetterSpacing
    color: properties.colorTextStrong
    verticalAlignment: Text.AlignVCenter

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }
}
