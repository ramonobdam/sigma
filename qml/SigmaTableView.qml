import QtQuick

// TableView component that adds functions for setting the current index and
// keeping track of the hovered row. Signals are emitted to request opening the
// delete delete dialog or add/edit window.
TableView {
    id: control

    property int hoveredRow: -1
    property string label: ""
    property string parameter: ""
    property string extra: ""
    property var upKeyFocusTarget: control
    property var downKeyFocusTarget: control
    property var leftKeyFocusTarget: control
    property var rightKeyFocusTarget: control

    // The deleteRequested sigmal is emitted when the delete key is pressed and
    // the table has at least one row
    signal deleteRequested()

    // The openRequested signal is usually connected to the same signal of the
    // context menu of the table delegate
    signal openRequested( edit: bool )

    // The extraRequested signal is usually connected to the same signal of the
    // context menu of the table delegate
    signal extraRequested()

    function determineHoveredRow () {
        for ( var row = 0; row < rows; ++row ) {
            for ( var column = 0; column < columns; ++column ) {
                let item = itemAtCell( Qt.point( column, row ) )
                if ( item !== null && item.hovered ) {
                    setHoveredRow( row )
                    return
                }
            }
        }
        setHoveredRow( -1 )
    }

    function setHoveredRow ( row ) {
        hoveredRow = row
    }

    function setCurrent( row, column ) {
        if ( selectionModel !== null) {
            selectionModel.setCurrentIndex( index( row, column ),
                                            ItemSelectionModel.Clear |
                                            ItemSelectionModel.Rows
                                           )
        }
    }

    // Overrule the arrow and (back)tab key presses for suitable current cell
    // positions. When the active focus is on a cell in the top row, left column
    // or right column, move the active focus to specified surrounding elements.
    // The delete key emits deleteRequested.
    Keys.onPressed: ( event ) => {
        if ( event.key === Qt.Key_Up ) {
            if ( activeFocus && currentRow === 0 ) {
                upKeyFocusTarget.forceActiveFocus()
                event.accepted = true
            }
            else {
                event.accepted = false
            }
            return
        }
        if ( event.key === Qt.Key_Down ) {
            if ( activeFocus && currentRow === ( rows - 1 ) ) {
                downKeyFocusTarget.forceActiveFocus()
                event.accepted = true
            }
            else {
                event.accepted = false
            }
            return
        }
        if ( event.key === Qt.Key_Right || event.key === Qt.Key_Tab ) {
            if ( activeFocus && currentColumn === ( columns - 1 ) ) {
                rightKeyFocusTarget.forceActiveFocus()
                event.accepted = true
            }
            else {
                event.accepted = false
            }
            return
        }
        if ( event.key === Qt.Key_Left || event.key === Qt.Key_Backtab ) {
            if ( activeFocus && currentColumn === 0 ) {
                leftKeyFocusTarget.forceActiveFocus()
                event.accepted = true
            }
            else {
                event.accepted = false
            }
            return
        }
        if ( event.key === Qt.Key_Delete ) {
            if ( activeFocus && rows > 0) {
                deleteRequested()
                event.accepted = true
            }
            else {
                event.accepted = false
            }
            return
        }
    }
}
