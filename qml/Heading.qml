// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Text heading component
Text {
    height: contentHeight
    font.family: Fonts.interSemiBold.font.family
    font.pixelSize: Properties.fontSizeHeader
    font.bold: true
    font.letterSpacing: Properties.headingLetterSpacing
    color: Properties.colorTextStrong
    verticalAlignment: Text.AlignVCenter
}
