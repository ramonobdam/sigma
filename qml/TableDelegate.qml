// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls

// Base table delegate component with context menu. The context menu type (
// input, output, correlation or default) is set based on Table.Type of the
// parent TableView.
Rectangle {
    id: control

    required property bool current
    required property string display
    required property int row
    required property int column
    readonly property TableView tv: TableView.view
    property var columnWidthProvider:
        function( column ) { return properties.defaultTableWidth }
    property bool hasFocus: current &&
                            tv.activeFocus &&
                            !properties.outputLocked
    property bool enableHover: true
    property bool currentRow: tv.currentRow === row
    property bool hoveredRow: tv.hoveredRow === row
    property string backgroundColor: properties.colorRaised
    property string label: ""
    property string parameter: ""
    property alias hovered: hoverHandler.hovered
    property alias properties: properties
    property alias fonts: fonts

    implicitWidth: columnWidthProvider( column )
    implicitHeight: properties.rowHeight + properties.borderWidth
    color: currentRow ? properties.colorBrand :
                        (
                            hoveredRow ? properties.colorTextHover :
                                         properties.colorTransparent
                        )
    clip: true

    ContextMenu.onRequested: position => {
        tv.setCurrent( row, column )
        let menu = contextMenu.createObject( control, { value: display } )
        menu.popup( position )
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
    }

    HoverHandler {
        id: hoverHandler

        enabled: control.enableHover
        onHoveredChanged: hovered ? tv.setHoveredRow ( row ) :
                                    tv.determineHoveredRow()
    }

    Component {
        id: contextMenu

        TableContextMenu {
            tableView: control.tv
            label: control.tv.label
            parameter: control.tv.parameter
            extra: control.tv.extra
        }
    }
}
