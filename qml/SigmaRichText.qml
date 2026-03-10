// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick

// Rich text component for HTML content. Hyperlinks are styled, open on
// activation and show the 'PointingHandCursor' on hover.
SigmaText {
    wrapMode: Text.WordWrap
    textFormat: Text.RichText
    onLinkActivated: function( link ) {
        Qt.openUrlExternally( link )
    }
    onLinkHovered: function( link) {
        if ( link ) {
            hoverHandler.cursorShape = Qt.PointingHandCursor
        } else {
            hoverHandler.cursorShape = Qt.ArrowCursor
        }
    }

    HoverHandler {
        id: hoverHandler
    }
}
