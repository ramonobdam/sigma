// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtCore
import Sigma

// Window component showing the undo history and allowing the user to restore
// states of the UndoStack
BaseWindow {
    id: control

    readonly property int undoCursor: calculation.undoCursor
    property alias tableView: undoHistoryTable.tableView

    function close() {
        control.hide()
    }

    function restoreState() {
        // Restore the selected state of the UndoStack
        let row = tableView.currentRow
        if ( row !== undoCursor && !Properties.outputLocked ) {
            calculation.restoreState( row )
        }
    }

    function syncSelection() {
        // Set the selected (i.e. current) row to the undoCursor position
        Qt.callLater(
            function() {
                historySelection.setCurrentIndex(
                    tableView.index( undoCursor, 0 ),
                    ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Rows
                )
            }
        )
    }

    function columnWidth( column ) {
        // A fixed column width is used for the Undo history table
        return  control.width -2 * container.anchors.margins
    }

    windowTitle: "Undo history"
    width: 400
    height: 500
    settingsKey: "undoHistoryWindow"
    settingsObject: settings
    onVisibleChanged: {
        if ( control.visible ) {
            undoHistoryTable.forceActiveFocus()
        }
    }

    // Keep selection in sync with the undoCursor
    Connections {
        target: calculation

        function onUndoCursorChanged() {
            control.syncSelection()
        }
    }

    Connections {
        target: tableView

        function onLayoutChanged() {
            control.syncSelection()
        }
    }

    // Restore the state when the currentRow changes
    Connections {
        target: historySelection

        function onCurrentRowChanged() {
            control.restoreState()
        }
    }

    ItemSelectionModel {
        id: historySelection
    }

    Settings {
        id: settings
    }

    Item {
        id: container

        anchors {
            left: parent.left
            right: parent.right
            top: titleBar.bottom
            bottom: parent.bottom
            margins: Properties.spacingM
        }

        focus: true
        Keys.onEscapePressed: { control.close() }

        FormSubHeading {
            // A line is used on top of the table instead of a header
            id: line

            anchors {
                left: parent.left
                top: parent.top
                topMargin: 0
            }
        }

        Table {
            id: undoHistoryTable

            anchors {
                top: line.bottom
                bottom: horizontalRule.top
                // To prevent a visual double line when the table is scrolled
                // all the way to the bottom the bottom margin is adjusted:
                bottomMargin: -horizontalRule.height
            }

            model: calculation.undoHistoryModel()
            selectionModel: historySelection
            showHeader: false
            resizableColumns: false
            delegate: DefaultTableDelegate {
                backgroundColor: control.color
                hasFocus: false
                enableContextMenu: false
                columnWidthProvider: control.columnWidth
            }
            upKeyFocusTarget: closeButton
            downKeyFocusTarget: undoButton.enabled ? undoButton :
                                                     (
                                                         redoButton.enabled ?
                                                             redoButton :
                                                             closeButton
                                                     )
        }

        FormSubHeading {
            id: horizontalRule

            anchors {
                bottom: undoButton.top
                bottomMargin: control.interFieldSpacing
            }
        }

        SecondaryButton {
            id: undoButton

            anchors {
                left: parent.left
                bottom: parent.bottom
            }

            text: Properties.buttonTextUndo
            enabled: Properties.canUndo

            onClicked: { calculation.undo() }

            KeyNavigation.backtab: undoHistoryTable
            KeyNavigation.tab: redoButton.enabled ? redoButton : closeButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        SecondaryButton {
            id: redoButton

            anchors {
                left: undoButton.right
                leftMargin: Properties.spacingButtons
                bottom: parent.bottom
            }

            text: Properties.buttonTextRedo
            enabled: Properties.canRedo

            onClicked: { calculation.redo() }

            KeyNavigation.backtab: undoButton.enabled ? undoButton :
                                                        undoHistoryTable
            KeyNavigation.tab: closeButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        SecondaryButton {
            id: closeButton

            anchors {
                right: parent.right
                bottom: undoButton.bottom
            }

            text: Properties.buttonTextClose

            onClicked: { control.close() }

            KeyNavigation.backtab:
                redoButton.enabled ? redoButton :
                                                (
                                                    undoButton.enabled ?
                                                        undoButton :
                                                        undoHistoryTable
                                                )
            KeyNavigation.tab: undoHistoryTable
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }
    }
}
