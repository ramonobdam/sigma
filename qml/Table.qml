import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Component that combines a HorizontalHeaderView and a TableView with scroll
FocusScope {
    id: control

    property bool empty: rows === 0
    property alias headerDelegate: headerView.delegate
    property alias model: tableView.model
    property alias delegate: tableView.delegate
    property alias selectionModel: tableView.selectionModel
    property alias columns: tableView.columns
    property alias rows: tableView.rows
    property alias tableView: tableView
    property alias tableViewFocus: tableView.focus
    property alias upKeyFocusTarget: tableView.upKeyFocusTarget
    property alias downKeyFocusTarget: tableView.downKeyFocusTarget
    property alias leftKeyFocusTarget: tableView.leftKeyFocusTarget
    property alias rightKeyFocusTarget: tableView.rightKeyFocusTarget
    property alias label: tableView.label
    property alias parameter: tableView.parameter
    property alias extra: tableView.extra

    signal openRequested( edit: bool )
    signal deleteRequested()
    signal extraRequested()

    anchors {
        left: parent.left
        right: parent.right
        top: parent.top
        bottom: parent.bottom
    }

    Connections {
        // Connections between Table signals and TableView signals
        target: tableView

        function onOpenRequested( edit ) { control.openRequested( edit ) }
        function onDeleteRequested() { control.deleteRequested() }
        function onExtraRequested() { control.extraRequested() }
    }

    SigmaProperties {
        id: properties
    }

    HorizontalHeaderView  {
        id: headerView

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        clip: true
        resizableColumns: true
        syncView: tableView
        boundsMovement: Flickable.StopAtBounds
    }

    ScrollView {
        anchors {
            top: headerView.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        clip: false
        focus: true
        ScrollBar.horizontal: SigmaScrollBar {
            anchors {
                top: parent.bottom
                topMargin: ( properties.spacingM - indicatorWidth ) / 2
                left: parent.left
                right: parent.right
            }
        }
        ScrollBar.vertical: SigmaScrollBar {
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.right
                leftMargin: ( properties.spacingM - indicatorWidth ) / 2
            }
        }

        SigmaTableView {
            id: tableView

            anchors.fill: parent

            clip: true
            focus: true
            resizableColumns: false
            selectionBehavior: TableView.SelectRows
            selectionMode: TableView.SingleSelection
            editTriggers: properties.outputLocked ? TableView.NoEditTriggers :
                          ( TableView.DoubleTapped | TableView.EditKeyPressed )
            boundsMovement: Flickable.StopAtBounds
        }
    }
}
