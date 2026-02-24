import QtQuick

// Styled horizontal rule component for context menus
Rectangle {
    id: control

    height: visible ? properties.borderWidth : 0
    color: properties.colorStrokeWeak

    SigmaProperties {
        id: properties
    }
}
