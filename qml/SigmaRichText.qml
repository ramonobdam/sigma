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
