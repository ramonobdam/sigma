import QtQuick
import QtQuick.Controls

// Context menu component for the table delegate
BaseContextMenu {
    id: control

    property string label: ""
    property string parameter: ""
    property string extra: ""
    property bool addNewOnly: false
    readonly property string space: parameter.length > 0 ? " " : ""
    readonly property bool labelAvailable: label.length > 0

    ContextMenuItem {
        id: addNewItem
        text: "Add new " + control.label + "..."
        enabled: !properties.outputLocked
        onTriggered: { control.openRequested( false ) }
        visible: control.labelAvailable
    }

    ContextMenuRule {
        visible: editItem.visible && addNewItem.visible
    }

    ContextMenuItem {
        id: editItem
        text: "Edit" + space + control.parameter + "..."
        enabled: !properties.outputLocked
        onTriggered: { control.openRequested( true ) }
        visible: !control.addNewOnly && control.labelAvailable
    }

    ContextMenuItem {
        id: deleteItem
        text: "Delete" + space + control.parameter + "..."
        enabled: !properties.outputLocked
        onTriggered: { control.deleteRequested() }
        visible: !control.addNewOnly && control.labelAvailable
    }

    ContextMenuRule {
        visible: copyItem.visible &&
                 ( deleteItem.visible || addNewItem.visible )
    }

    ContextMenuItem {
        id: copyItem
        text: "Copy value '" + control.value + "'"
        onTriggered: { control.copyToClipboard( control.value ) }
        visible: control.value.length > 0
    }

    ContextMenuRule {
        visible: extraItem.visible &&
                 (
                     addNewItem.visible ||
                     deleteItem.visible ||
                     copyItem.visible
                 )
    }

    ContextMenuItem {
        id: extraItem
        text: control.extra
        enabled: !properties.outputLocked
        onTriggered: { control.extraRequested() }
        visible: control.extra.length > 0
    }
}
