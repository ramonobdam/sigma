// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import QtQuick.Effects

// Base Context Menu component. Its signals are connected to tableView if it is
// defined.
Menu {
    id: control

    property string value
    property string backgroundColor: properties.colorElevated
    property TableView tableView
    property alias properties: properties

    signal openRequested( edit: bool )
    signal deleteRequested()
    signal extraRequested()

    Connections {
        // Connections between ContextMenu signals and TableView signals
        target: control

        function onOpenRequested( edit ) {
            if ( tableView ) {
                tableView.openRequested( edit )
            }
        }
        function onDeleteRequested() {
            if ( tableView ) {
                tableView.deleteRequested()
            }
        }
        function onExtraRequested() {
            if ( tableView ) {
                tableView.extraRequested()
            }
        }
    }

    function copyToClipboard( value ) {
        // Copy a value to the clipboard using a hidden TextEdit
        clipboard.text = value
        clipboard.selectAll()
        clipboard.copy()
    }

    width: properties.contextMenuWidth
    padding: bg.border.width
    verticalPadding: bg.border.width

    TextEdit {
        // Hidden TextEdit to facilitate copying to clipboard in QML.
        id: clipboard
        visible: false
        height: 0
        width: 0
    }

    SigmaProperties {
        id: properties
    }

    background: Rectangle {
        id: bg
        color: properties.colorElevated
        radius: properties.radiusS
        border.width: properties.borderWidth
        border.color: properties.colorStrokeWeak
        layer.enabled: true
        layer.effect: RoundedElevationEffect {
            elevation: properties.elevationElevated
            roundedScale: Material.ExtraSmallScale
        }
    }

    contentItem: ListView {
        id: listView

        implicitHeight: contentHeight
        model: control.contentModel
        interactive: Window.window
                     ? contentHeight + control.topPadding + control.bottomPadding > control.height
                     : false
        clip: true
        currentIndex: control.currentIndex

        ScrollIndicator.vertical: SigmaScrollIndicator {}

        // Clip the rounded corners of the background by using a MultiEffect
        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: mask
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
        }

        // No context menu on the context menu itself
        ContextMenu.onRequested: {}

        Rectangle {
            id: mask
            width: listView.width
            height: listView.height
            radius: bg.radius - bg.border.width
            visible: false
            layer.enabled: true
            layer.smooth: true
        }
    }
}
