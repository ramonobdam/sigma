// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtCore
import Sigma

// Window component showing the correlations table
BaseWindow {
    id: control

    property bool correlationAvailable: correlationsTable.rows > 0
    property TableView tableView

    function close() {
        control.hide()
    }

    function openDelete() {
        if ( control.visible && control.correlationAvailable ) {
            deleteCorrelation.show()
        }
    }

    function openCorrelation( edit ) {
        editCorrelation.open( edit )
    }

    function getCorrelationColumnWidth( column ) {
        return roundToDevicePixels( settings.correlationColumnWidths[ column ] )
    }

    function storeCorrelationColumnWidths( tableView ) {
        for ( let i = 0;  i < tableView.columns; ++i ) {
            if ( tableView.columnWidth( i ) > 0 ) {
                settings.correlationColumnWidths[ i ] = tableView.columnWidth( i )
            }
        }
    }

    function roundToDevicePixels( value ) {
        // Round the value in device independent pixels (DIP) to the nearest
        // value in device pixels (DPI) to prevent pixel rounding issues when
        // dpi-scaling is applied.
        let valueDPIrounded = Math.round( value * Screen.devicePixelRatio )
        return valueDPIrounded / Screen.devicePixelRatio
    }

    windowTitle: "Input parameter correlations"
    width: 500
    height: 420
    settingsKey: "correlationsWindow"
    settingsObject: settings
    onVisibleChanged: {
        if ( control.visible ) {
            addCorrelationButton.forceActiveFocus()
        }
    }

    Settings {
        // Store persistent settings
        id: settings

        // Table column widths:
        property list<real> correlationColumnWidths:
            calculation.getCorrelationColumnWidths()
    }

    SigmaProperties {
        id: properties
    }

    CorrelationEditWindow {
        id: editCorrelation

        tableView: correlationsTable.tableView
    }

    DeleteCorrelationDialog {
        id: deleteCorrelation

        transientParent: control
    }

    Item {
        id: container

        anchors {
            left: parent.left
            right: parent.right
            top: titleBar.bottom
            bottom: parent.bottom
            margins: properties.spacingM
        }

        focus: true
        ContextMenu.menu: TableContextMenu {
            label: "correlation"
            addNewOnly: true
            onOpenRequested: ( edit ) => control.openCorrelation( edit )
        }
        Keys.onEscapePressed: { control.close() }

        Heading {
            id: correlationsHeading

            anchors {
                topMargin: 0
            }

            text: "Correlations"
        }

        Item {
            id: buttons

            anchors {
                top: parent.top
                right: parent.right
                // Compensate for 'activefocus border' on button
                topMargin: -2 * properties.borderWidth
                rightMargin: -2 * properties.borderWidth
            }

            SecondaryButton {
                id: addCorrelationButton

                text: properties.buttonTextNew
                toolTipText: enabled ? properties.tipNewCorrelation :
                                       properties.tipAdd2InputParamFirst
                anchors {
                    right: parent.left
                }
                enabled: control.tableView.rows > 1
                onClicked: { control.openCorrelation( false ) }
                KeyNavigation.backtab: closeButton
                KeyNavigation.tab: control.correlationAvailable ?
                                       correlationsTable :
                                       closeButton
                KeyNavigation.up: KeyNavigation.backtab
                KeyNavigation.down: KeyNavigation.tab
            }
        }

        Table {
            id: correlationsTable

            anchors {
                top: correlationsHeading.bottom
                topMargin: properties.spacingM
                bottom: horizontalRule.top
                bottomMargin: properties.spacingM
            }

            model: calculation.correlationItemModel()
            selectionModel: calculation.correlationSelectionModel()
            headerDelegate: HeaderDelegate {
                columnWidthProvider:
                    function( column ) {
                        return control.getCorrelationColumnWidth( column )
                    }
            }
            delegate: DefaultTableDelegate {
                backgroundColor: control.color
                columnWidthProvider:
                    function( column ) {
                        return control.getCorrelationColumnWidth( column )
                    }
                TableView.editDelegate: Component {
                    Item {
                        Component.onCompleted: { openCorrelation( true ) }
                    }
                }
            }
            label: "correlation"
            onDeleteRequested: { control.openDelete() }
            onOpenRequested: ( edit ) => control.openCorrelation( edit )
            upKeyFocusTarget: addCorrelationButton
            downKeyFocusTarget: closeButton
            Component.onDestruction:
                control.storeCorrelationColumnWidths( tableView )
        }

        FormSubHeading {
            id: horizontalRule

            anchors {
                bottom: closeButton.top
                bottomMargin: control.interFieldSpacing
            }
        }

        SecondaryButton {
            id: closeButton

            anchors {
                bottom: parent.bottom
                left: parent.left
            }

            text: "Close"
            onClicked: { control.close() }

            KeyNavigation.backtab:
                control.correlationAvailable ? correlationsTable :
                                               addCorrelationButton
            KeyNavigation.tab: addCorrelationButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }
    }
}
