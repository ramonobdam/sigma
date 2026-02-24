import QtQuick
import QtQuick.Controls
import Sigma

// Context menu component for the output parameters table delegate
BaseContextMenu {
    id: control

    ContextMenuItem {
        text: "Add new output parameter..."
        enabled: !properties.outputLocked
        onTriggered: control.tableView.openRequested( false )
    }

    Rectangle {
        height: properties.borderWidth
        color: properties.colorStrokeWeak
    }

    ContextMenuItem {
        text: "Edit " + properties.outputName + "..."
        enabled: !properties.outputLocked
        onTriggered: control.tableView.openRequested( true )
    }

    ContextMenuItem {
        text: "Delete " + properties.outputName + "..."
        enabled: !properties.outputLocked
        onTriggered: control.tableView.deleteRequested()
    }

    Rectangle {
        height: copyItem.visible ? properties.borderWidth : 0
        color: properties.colorStrokeWeak
    }

    ContextMenuItem {
        id: copyItem
        visible: control.value.length > 0
        text: "Copy value '" + control.value + "'"
        onTriggered: copyToClipboard( control.value )
    }
}
