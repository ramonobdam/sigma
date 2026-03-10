// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import Sigma

// Context menu component for Monte Carlo results table
BaseContextMenu {
    id: control

    property Item table

    Connections {
        // Connections between ContextMenu signals and TableView signals
        target: control

        function onExtraRequested() {
            if ( table ) {
                table.extraRequested()
            }
        }
    }

    ContextMenuItem {
        text: "Run Monte Carlo simulation"
        enabled: calculation.outputValid && !properties.outputLocked
        onTriggered: { calculation.runMonteCarlo() }
    }

    ContextMenuItem {
        text: "Stop Monte Carlo simulation"
        enabled: calculation.outputValid && properties.outputLocked
        onTriggered: { calculation.stopMonteCarlo() }
    }

    Rectangle {
        height: properties.borderWidth
        color: properties.colorStrokeWeak
    }

    ContextMenuItem {
        id: copyItem
        visible: control.value.length > 0
        text: "Copy value '" + control.value + "'"
        onTriggered: { copyToClipboard( control.value ) }
    }

    Rectangle {
        height: copyItem.visible ? properties.borderWidth : 0
        color: properties.colorStrokeWeak
    }

    ContextMenuItem {
        text: "Export results to CSV file..."
        onTriggered: { control.extraRequested() }
        enabled: !properties.outputLocked
    }
}
