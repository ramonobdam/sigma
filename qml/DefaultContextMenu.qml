import Sigma
import QtQuick
import QtQuick.Controls

// Default context menu component
BaseContextMenu {
    id: control

    ContextMenuItem {
        id: copyItem
        visible: control.value.length > 0
        text: "Copy value '" + control.value + "'"
        onTriggered: copyToClipboard( value )
    }

    Rectangle {
        height: copyItem.visible ? properties.borderWidth : 0
        color: properties.colorStrokeWeak
    }

    ContextMenuItem {
        text: "Export results to CSV file..."
        onTriggered: csvDialog.open()
        enabled: !properties.outputLocked
    }
}
