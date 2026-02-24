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
