import QtQuick
import QtQuick.Controls

// Styled horizontal rule component for (context) menus
MenuSeparator {
    id: control

    padding: 0
    verticalPadding: 0

    contentItem: Rectangle {
        implicitHeight: visible ? properties.borderWidth : 0
        color: properties.colorStrokeWeak
    }

    SigmaProperties {
        id: properties
    }
}
