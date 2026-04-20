// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import QtCore
import QtQml.Models
import QtGraphs
import Sigma

ApplicationWindow {
    id: window

    property int titleBarHeight:
        Math.max( SafeArea.margins.top, Properties.minimumTitleBarHeight )

    function saveProjectAs( closeWindowOnSuccess = false ) {
        fileDialog.fileMode = FileDialog.SaveFile
        fileDialog.closeWindowOnSuccess = closeWindowOnSuccess
        fileDialog.open()
    }

    function saveProject( closeWindowOnSuccess = false) {
        let success = calculation.saveProject()
        if ( success && closeWindowOnSuccess ) {
            closeWindow()
        }
        else if ( !success ) {
            projectFailed.save = true
            projectFailed.show()
        }
    }

    function openProject( checkForUnsavedChanges = true ) {
        if ( checkForUnsavedChanges && Properties.unsavedChanges ) {
            // Saving changes before opening a project
            discardUnsavedChanges.mode = DiscardUnsavedChanges.OpenProject
            discardUnsavedChanges.show()
        }
        else {
            fileDialog.fileMode = FileDialog.OpenFile
            fileDialog.open()
        }
    }

    function newProject( checkForUnsavedChanges = true ) {
        if ( checkForUnsavedChanges && Properties.unsavedChanges ) {
            // Saving changes before creating a new project
            discardUnsavedChanges.mode = DiscardUnsavedChanges.NewProject
            discardUnsavedChanges.show()
        }
        else {
            calculation.newProject()
            saveProjectAs()
        }
    }

    function roundToDevicePixels( value ) {
        // Round the value in device independent pixels (DIP) to the nearest
        // value in device pixels (DPI) to prevent pixel rounding issues when
        // dpi-scaling is applied.
        let valueDPIrounded = Math.round( value * Screen.devicePixelRatio )
        return valueDPIrounded / Screen.devicePixelRatio
    }

    // The following functions obtain and store the table column widths from the
    // persistent settings
    function getInputColumnWidth( column ) {
        return roundToDevicePixels( settings.inputColumnWidths[ column ] )
    }

    function storeInputColumnWidths( tableView ) {
        for ( let i = 0;  i < tableView.columns; ++i ) {
            if ( tableView.columnWidth( i ) > 0 ) {
                settings.inputColumnWidths[ i ] = tableView.columnWidth( i )
            }
        }
    }

    function getOutputColumnWidth( column ) {
        return roundToDevicePixels( settings.outputColumnWidths[ column ] )
    }

    function storeOutputColumnWidths( tableView ) {
        for ( let i = 0;  i < tableView.columns; ++i ) {
            if ( tableView.columnWidth( i ) > 0 ) {
                settings.outputColumnWidths[ i ] = tableView.columnWidth( i )
            }
        }
    }

    function getResultsColumnWidth( column ) {
        return roundToDevicePixels( settings.resultsColumnWidths[ column ] )
    }

    function storeResultsColumnWidths( tableView ) {
        for ( let i = 0;  i < tableView.columns; ++i ) {
            if ( tableView.columnWidth( i ) > 0 ) {
                settings.resultsColumnWidths[ i ] = tableView.columnWidth( i )
            }
        }
    }

    function getBudgetColumnWidth( column ) {
        return roundToDevicePixels( settings.budgetColumnWidths[ column ] )
    }

    function storeBudgetColumnWidths( tableView ) {
        for ( let i = 0;  i < tableView.columns; ++i ) {
            if ( tableView.columnWidth( i ) > 0 ) {
                settings.budgetColumnWidths[ i ] = tableView.columnWidth( i )
            }
        }
    }

    function openInputParam( edit ) {
        inputParamWindow.open( edit )
    }

    function openOutputParam( edit ) {
        outputParamWindow.open( edit )
    }

    function deleteInputParam() {
        deleteInputParamDialog.show()
    }

    function deleteOutputParam() {
        deleteOutputParamDialog.show()
    }

    function openCorrelations() {
        correlationsWindow.show()
    }

    function openCSV() {
        csvDialog.open()
    }

    function openSettings() {
        settingsWindow.show()
    }

    function closeWindow() {
        window.close()
    }

    function clearProject() {
        clearDialog.show()
    }

    function openAbout() {
        aboutWindow.show()
    }

    function undo() {
        calculation.undo()
    }

    function redo() {
        calculation.redo()
    }

    width: Properties.defaultMainWindowWidth
    height: Properties.defaultMainWindowHeight
    topPadding: 0
    minimumWidth: Properties.minWidthParam +
                  Properties.spacingM +
                  Properties.minWidthMonteCarlo +
                  splitViewMain.anchors.margins * 2
    minimumHeight: Properties.minHeightParam +
                   Properties.spacingM +
                   Properties.minHeightResults +
                   splitViewMain.anchors.margins * 2
    visible: true
    title: Properties.macOS ? "" : Properties.windowTitle
    flags: Properties.windowFlags
    color: Properties.colorBase
    font.family: Fonts.inter.font.family
    font.pixelSize: Properties.fontSizeBody
    Material.theme: Properties.lightTheme ? Material.Light : Material.Dark
    onClosing: ( close ) => {
        // Before closing the main window:

        // Stop Monte Carlo simulation in case it is running
        if ( Properties.outputLocked ) {
            calculation.stopMonteCarlo()
        }

        // Open 'discard unsaved changes?' dialog if needed
        if ( Properties.unsavedChanges && !discardUnsavedChanges.closeWindow ) {
            close.accepted = false
            discardUnsavedChanges.mode = DiscardUnsavedChanges.Close
            discardUnsavedChanges.show()
        }
    }
    Component.onCompleted: {
        // Create native framesless window on Windows
        captionHelper.attachTo( window )
        captionHelper.captionHeight = window.titleBarHeight
    }

    Settings {
        // Store persistent settings
        id: settings

        // Main window position and dimensions:
        property alias x: window.x
        property alias y: window.y
        property alias width: window.width
        property alias height: window.height

        // Selected files:
        property alias projectFile: fileDialog.selectedFile
        property alias csvSelectedFile: csvDialog.selectedFile

        // SplitView states:
        property var svMain
        property var svVertical
        property var svParam

        // Table column widths:
        property list<real> inputColumnWidths: calculation.getInputColumnWidths()
        property list<real> outputColumnWidths:
            calculation.getOutputColumnWidths()
        property list<real> resultsColumnWidths:
            calculation.getResultsColumnWidths()
        property list<real> budgetColumnWidths:
            calculation.getBudgetColumnWidths()
        property list<real> correlationColumnWidths:
            calculation.getCorrelationColumnWidths()
    }

    SigmaTitleBar {
        id: titleBar

        title: Properties.windowTitle
        titleBarHeight: window.titleBarHeight
        titleBarLeftMargin: Properties.macOS ?
                                Properties.titleBarTextLeftMargin :
                                (
                                    icon.anchors.leftMargin +
                                    icon.width +
                                    menuBar.anchors.leftMargin +
                                    menuBar.implicitWidth +
                                    Properties.spacingM
                                )
        titleAlignment: Text.AlignHCenter
        color: window.color

        Component.onCompleted: {
            registerItem( menuBar )
        }

        Image {
            id: icon

            anchors {
                left: parent.left
                leftMargin: ( parent.height - height ) / 2
                verticalCenter: parent.verticalCenter
            }

            visible: Properties.windows
            source: Properties.appIcon
            width: visible ? Properties.titleBarIconWidth : 0
            height: width
            fillMode: Image.PreserveAspectFit
            mipmap: true
            antialiasing: true
        }

        SigmaMenuBar {
            id: menuBar

            anchors {
                left: icon.right
                leftMargin: Properties.spacingXS
            }

            property bool fullScreen:
                window.visibility === Window.FullScreen
            property bool minimized: window.visibility === Window.Minimized
            property bool maximized: window.visibility === Window.Maximized
            property bool windowed: window.visibility === Window.Windowed

            SigmaMenu {
                title: Application.name
                implicitWidth: 220

                SigmaMenuBarMenuItem {
                    // Moved to Application Menu on Mac by the OS
                    text: "Settings..."
                    onTriggered: { openSettings() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    // Moved to Application Menu on Mac by the OS
                    text: "About " + Application.name + "..."
                    onTriggered: { openAbout() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    // Moved to Application Menu on Mac by the OS
                    text: Properties.macOs ? "Quit" : "Exit"
                    shortcut: Properties.macOs ? StandardKey.Close : "Alt+F4"
                    onTriggered: { closeWindow() }
                }
            }

            SigmaMenu {
                title: "Project"
                implicitWidth: 320

                SigmaMenuBarMenuItem {
                    text: "New..."
                    shortcut: Properties.macOs ? StandardKey.New : "Ctrl+N"
                    enabled: !Properties.outputLocked
                    onTriggered: { newProject() }
                }

                SigmaMenuBarMenuItem {
                    text: "Open..."
                    shortcut: Properties.macOs ? StandardKey.Open : "Ctrl+O"
                    enabled: !Properties.outputLocked
                    onTriggered: { openProject() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Save"
                    shortcut: Properties.macOs ? StandardKey.Save : "Ctrl+S"
                    enabled: !Properties.outputLocked &&
                             Properties.unsavedChanges
                    onTriggered: {
                        Properties.filePathNotEmpty ? saveProject() :
                                                      saveProjectAs()
                    }
                }

                SigmaMenuBarMenuItem {
                    text: "Save As..."
                    shortcut: Properties.macOs ? StandardKey.SaveAs :
                                                 "Ctrl+Shift+S"
                    enabled: !Properties.outputLocked
                    onTriggered: { saveProjectAs() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Auto Save"
                    checkable: true
                    checked: Properties.autoSaveProject
                    onTriggered: {
                        appSettings.setAutoSaveProject( this.checked )
                        // When Auto Save is switched on, save any unsaved changes
                        if ( this.checked && Properties.unsavedChanges ) {
                            calculation.saveProject()
                        }
                    }
                }

                SigmaMenuBarMenuItem {
                    text: "Restore last project on startup"
                    checkable: true
                    checked: Properties.restoreLastProject
                    onTriggered: { appSettings.setRestoreLastProject( this.checked ) }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Clear..."
                    shortcut: "Ctrl+Delete"
                    onTriggered: { clearProject() }
                    enabled: !Properties.outputLocked
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Export results to CSV file..."
                    shortcut: "Ctrl+E"
                    onTriggered: { openCSV() }
                    enabled: !Properties.outputLocked
                }
            }

            SigmaMenu {
                title: "Edit"
                implicitWidth: 230

                SigmaMenuBarMenuItem {
                    text: "Undo"
                    shortcut: Properties.macOs ? StandardKey.Undo : "Ctrl+Z"
                    enabled: Properties.canUndo
                    onTriggered: { undo() }
                }

                SigmaMenuBarMenuItem {
                    text: "Redo"
                    shortcut: Properties.macOs ? StandardKey.Redo : "Ctrl+Y"
                    enabled: Properties.canRedo
                    onTriggered: { redo() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Clear project..."
                    shortcut: "Ctrl+Delete"
                    onTriggered: { clearProject() }
                    enabled: !Properties.outputLocked
                }
            }

            SigmaMenu {
                title: "Input parameters"
                implicitWidth: 270

                SigmaMenuBarMenuItem {
                    text: "Add new..."
                    shortcut: "Ctrl+Shift+I"
                    enabled: !Properties.outputLocked
                    onTriggered: { openInputParam( false ) }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Edit" +
                          (
                               Properties.inputParamAvailable ?
                                  ( " " + Properties.inputName ) :
                                   ""
                          )
                          + "..."
                    enabled: !Properties.outputLocked &&
                             Properties.inputParamAvailable
                    onTriggered:  { openInputParam( true ) }
                }

                SigmaMenuBarMenuItem {
                    text: "Delete" +
                          (
                              Properties.inputParamAvailable ?
                                  ( " " + Properties.inputName ) :
                                  ""
                          ) +
                          "..."
                    enabled: !Properties.outputLocked &&
                             Properties.inputParamAvailable
                    onTriggered: { deleteInputParam() }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Correlations..."
                    shortcut: "Ctrl+Shift+C"
                    enabled: !Properties.outputLocked
                    onTriggered: { openCorrelations() }
                }
            }

            SigmaMenu {
                title: "Output parameters"
                implicitWidth: 270

                SigmaMenuBarMenuItem {
                    text: "Add new..."
                    shortcut: "Ctrl+Shift+O"
                    enabled: !Properties.outputLocked
                    onTriggered: { openOutputParam( false ) }
                }

                ContextMenuRule {}

                SigmaMenuBarMenuItem {
                    text: "Edit" +
                          (
                              Properties.outputParamAvailable ?
                                  ( " " + Properties.outputName ) :
                                  ""
                          ) +
                          "..."
                    enabled: !Properties.outputLocked &&
                             Properties.outputParamAvailable
                    onTriggered: { openOutputParam( true ) }
                }

                SigmaMenuBarMenuItem {
                    text: "Delete" +
                          (
                              Properties.outputParamAvailable ?
                                  ( " " + Properties.outputName ) :
                                  ""
                          )
                          + "..."
                    enabled: !Properties.outputLocked &&
                             Properties.outputParamAvailable
                    onTriggered: { deleteOutputParam() }
                }
            }

            SigmaMenu {
                title: "Monte Carlo simulation"
                implicitWidth: 170

                SigmaMenuBarMenuItem {
                    text: "Start"
                    shortcut: "Ctrl+R"
                    enabled: calculation.outputValid && !Properties.outputLocked
                    onTriggered: calculation.runMonteCarlo()
                }

                SigmaMenuBarMenuItem {
                    text: "Stop"
                    shortcut: "Ctrl+T"
                    enabled: calculation.outputValid && Properties.outputLocked
                    onTriggered: calculation.stopMonteCarlo()
                }
            }

            SigmaMenu {
                title: "Window"
                implicitWidth: 220

                SigmaMenuBarMenuItem {
                    text: "Minimize"
                    shortcut: "Ctrl+M"
                    enabled: !menuBar.minimized
                    onTriggered: { window.showMinimized() }
                }

                SigmaMenuBarMenuItem {
                    text: Properties.macOS ?
                                "Zoom" :
                                ( menuBar.maximized ? "Restore" : "Maximize" )
                    enabled: !menuBar.fullScreen
                    onTriggered: { captionHelper.toggleMaximize( window ) }
                }


                SigmaMenuBarMenuItem {
                    text: "Full Screen"
                    checkable: true
                    checked: menuBar.fullScreen
                    shortcut: Properties.macOS ? StandardKey.FullScreen : "F11"
                    onTriggered: { captionHelper.toggleFullScreen( window ) }
                }
            }
        }
    }

    SigmaSplitView {
        id: splitViewMain
        // Horizontal splitview with parameters and results on the left side and
        // Monte Carlo on the right side

        anchors {
            left: parent.left
            right: parent.right
            top: titleBar.bottom
            bottom: parent.bottom
            margins: Properties.spacingM
            topMargin: ( titleBar.height === 0 ) ? Properties.spacingM : 0
        }

        orientation: Qt.Horizontal
        Component.onCompleted: restoreState( settings.svMain )
        Component.onDestruction: settings.svMain = saveState()

        SigmaSplitView {
            id: splitViewVertical
            // Vertical split view with input/output parameters on the top and
            // results on the bottom

            orientation: Qt.Vertical
            SplitView.minimumWidth: Properties.minWidthParam
            SplitView.preferredWidth: Properties.preferredWidthParam
            Component.onCompleted: restoreState( settings.svVertical )
            Component.onDestruction: settings.svVertical = saveState()

            SigmaSplitView {
                id: splitViewParam
                // Horizontal split view with input parameters on the left and
                // output parameters on the right

                orientation: Qt.Horizontal
                SplitView.minimumHeight: Properties.minHeightParam
                SplitView.preferredHeight: Properties.preferredHeightParam
                Component.onCompleted: restoreState( settings.svParam )
                Component.onDestruction: settings.svParam = saveState()

                Item {
                    // Input parameters panel

                    SplitView.minimumWidth: Properties.minWidthInputParam
                    SplitView.preferredWidth:
                        Properties.preferredWidthInputParam

                    Card {
                        id: inputParamCard

                        title: "Input parameters"
                        ContextMenu.menu: TableContextMenu {
                            label: "input parameter"
                            extra: "Correlations..."
                            addNewOnly: true
                            onOpenRequested: ( edit ) => openInputParam( edit )
                            onExtraRequested: { openCorrelations() }
                        }

                        Item {
                            id: inputParamButtons

                            anchors {
                                top: parent.top
                                right: parent.right
                                // Compensate for 'activefocus border' on button
                                topMargin: -2 * Properties.borderWidth
                                rightMargin: -2 * Properties.borderWidth
                            }

                            SecondaryButton {
                                id: addInputParamButton

                                text: Properties.buttonTextNew
                                backgroundColor: Properties.colorRaised
                                toolTipText: Properties.tipNewInputParameter
                                toolTipTextDisabled:
                                    Properties.tipMonteCarloRunning
                                enabled: !Properties.outputLocked
                                anchors {
                                    right: correlationsButton.left
                                    rightMargin: Properties.spacingButtons
                                }
                                onClicked: { openInputParam( false ) }
                                KeyNavigation.backtab: runMonteCarloButton
                                KeyNavigation.tab: correlationsButton
                                KeyNavigation.up: inputTable
                                KeyNavigation.down: inputTable
                                Component.onCompleted: forceActiveFocus()
                            }

                            SecondaryButton {
                                id: correlationsButton

                                text: "Correlations"
                                backgroundColor: Properties.colorRaised
                                toolTipText: Properties.tipShowCorrelations
                                toolTipTextDisabled:
                                    Properties.outputLocked ?
                                        Properties.tipMonteCarloRunning :
                                        Properties.tipAdd2InputParamFirst
                                anchors {
                                    right: parent.right
                                }
                                enabled: !Properties.outputLocked
                                onClicked: { openCorrelations() }
                                KeyNavigation.backtab: addInputParamButton
                                KeyNavigation.tab: addOutputParamButton
                                KeyNavigation.up: inputTable
                                KeyNavigation.down: inputTable
                            }
                        }

                        Table {
                            id: inputTable

                            anchors {
                                top: inputParamCard.header.bottom
                            }
                            model: calculation.inputItemModel()
                            selectionModel: calculation.inputSelectionModel()
                            headerDelegate: HeaderDelegate {
                                columnWidthProvider:
                                    function( column ) {
                                        return getInputColumnWidth( column )
                                    }
                            }
                            delegate: DefaultTableDelegate {
                                columnWidthProvider:
                                    function( column ) {
                                        return getInputColumnWidth( column )
                                    }
                                TableView.editDelegate: Component {
                                    Item {
                                        Component.onCompleted:
                                            inputParamWindow.open( true )
                                    }
                                }
                            }
                            label: "input parameter"
                            parameter: Properties.inputName
                            extra: "Correlations..."
                            onDeleteRequested: { deleteInputParam() }
                            onOpenRequested: ( edit ) => openInputParam( edit )
                            onExtraRequested: { openCorrelations() }
                            upKeyFocusTarget: addInputParamButton
                            rightKeyFocusTarget: outputTable
                            leftKeyFocusTarget: runMonteCarloButton
                            Component.onDestruction:
                                storeInputColumnWidths( tableView )
                        }
                    }
                }

                Item {
                    // Output parameters panel

                    SplitView.minimumWidth: Properties.minWidthOutputParam
                    SplitView.fillWidth: true

                    Card {
                        id: outputParamCard

                        title: Properties.titleOutputParameters
                        ContextMenu.menu: TableContextMenu {
                            label: "output parameter"
                            addNewOnly: true
                            onOpenRequested: ( edit ) => openOutputParam( edit )
                        }

                        Item {
                            id: outputParamButtons

                            anchors {
                                top: parent.top
                                right: parent.right
                                // Compensate for 'activefocus border' on button
                                topMargin: -2 * Properties.borderWidth
                                rightMargin: -2 * Properties.borderWidth
                            }

                            SecondaryButton {
                                id: addOutputParamButton

                                text: Properties.buttonTextNew
                                backgroundColor: Properties.colorRaised
                                toolTipText: Properties.tipNewOutputParameter
                                toolTipTextDisabled:
                                    Properties.tipMonteCarloRunning
                                enabled: !Properties.outputLocked
                                anchors {
                                    right: parent.right
                                }
                                onClicked: openOutputParam( false )
                                KeyNavigation.backtab: correlationsButton
                                KeyNavigation.tab: runMonteCarloButton
                                KeyNavigation.up: outputTable
                                KeyNavigation.down: outputTable
                            }
                        }

                        Table {
                            id: outputTable

                            anchors {
                                top: outputParamCard.header.bottom
                            }
                            model: calculation.outputItemModel()
                            selectionModel: calculation.outputSelectionModel()
                            headerDelegate: HeaderDelegate {
                                // The first column header delegate shows the
                                // (in)valid icon
                                useValid: column === 0

                                columnWidthProvider:
                                    function( column ) {
                                        return getOutputColumnWidth( column )
                                }
                            }
                            delegate: DefaultTableDelegate {
                                // The first column table delegate shows the
                                // (in)valid icon
                                useValid: column === 0
                                columnWidthProvider:
                                    function( column ) {
                                        return getOutputColumnWidth( column )
                                    }
                                TableView.editDelegate: Component {
                                    Item {
                                        Component.onCompleted:
                                            openOutputParam( true )
                                    }
                                }
                            }
                            label: "output parameter"
                            parameter: Properties.outputName
                            onDeleteRequested: { deleteOutputParam() }
                            onOpenRequested: ( edit ) => openOutputParam( edit )
                            upKeyFocusTarget: addOutputParamButton
                            rightKeyFocusTarget: runMonteCarloButton
                            leftKeyFocusTarget: inputTable
                            Component.onDestruction:
                                storeOutputColumnWidths( tableView )
                        }
                    }
                }
            }

            Item {
                // Results panel: combined uncertainty results and uncertainty
                // components

                SplitView.minimumHeight: Properties.minHeightResults
                SplitView.fillHeight: true

                Card {
                    id: resultsCard

                    title: Properties.titleCombinedUncertainty
                    ContextMenu.menu: TableContextMenu {
                        extra: "Export results to CSV file..."
                        onExtraRequested: { openCSV() }
                    }

                    Table {
                        id: resultsTable

                        anchors {
                            top: resultsCard.header.bottom
                            left: parent.left
                            right: parent.right
                            bottom: undefined
                        }
                        height: (
                                    Properties.rowHeight +
                                    Properties.borderWidth
                                ) * 2
                        model: calculation.resultsItemModel()
                        delegate: DefaultTableDelegate {
                            enableHover: false
                            columnWidthProvider:
                                function( column ) {
                                    return getResultsColumnWidth( column )
                                }
                        }
                        headerDelegate: HeaderDelegate {
                            columnWidthProvider:
                                function( column ) {
                                    return getResultsColumnWidth( column )
                                }
                        }
                        extra: "Export results to CSV file..."
                        onExtraRequested: { openCSV() }
                        Component.onDestruction:
                            storeResultsColumnWidths( tableView )
                    }

                    Heading {
                        id: budgetHeader

                        text: Properties.titleUncertaintyComponents
                        anchors {
                            top: resultsTable.bottom
                            topMargin:
                                Properties.spacingM + Properties.spacingXS
                            left: parent.left
                            right: parent.right
                        }
                    }

                    Table {
                        id: budgetTable

                        anchors {
                            top: budgetHeader.bottom
                            topMargin: Properties.spacingM
                            left: parent.left
                            right: parent.right
                        }
                        model: calculation.budgetItemModel()
                        delegate: DefaultTableDelegate {
                            // The last column table delegate shows the color
                            // scale of the total contribution
                            useScale:
                                column === budgetTable.tableView.columns - 1
                            enableHover: false
                            columnWidthProvider:
                                function( column ) {
                                    return window.getBudgetColumnWidth( column )
                                }
                        }
                        headerDelegate: HeaderDelegate {
                            columnWidthProvider:
                                function( column ) {
                                    return window.getBudgetColumnWidth( column )
                                }
                        }
                        extra: "Export results to CSV file..."
                        onExtraRequested: { openCSV() }
                        Component.onDestruction:
                            storeBudgetColumnWidths( tableView )
                    }
                }
            }
        }

        Item {
            // Monte Carlo panel

            SplitView.minimumWidth: Properties.minWidthMonteCarlo
            SplitView.fillWidth: true

            Card {
                id: monteCarloCard

                title: Properties.titleMonteCarlo
                ContextMenu.menu: MonteCarloContextMenu {
                    onExtraRequested: { openCSV() }
                }

                Item {
                    id: monteCarloButton

                    anchors {
                        top: parent.top
                        right: parent.right
                        // Compensate for 'active focus border' on button
                        topMargin: -2 * Properties.borderWidth
                        rightMargin: -2 * Properties.borderWidth
                    }

                    SecondaryButton {
                        id: runMonteCarloButton

                        anchors {
                            right: parent.right
                        }
                        width: 62
                        text: !Properties.outputLocked ?
                                  Properties.buttonTextStart :
                                  Properties.buttonTextStop
                        backgroundColor: Properties.colorRaised
                        toolTipText:
                            !Properties.outputLocked ?
                                Properties.tipStartMonteCarlo :
                                Properties.tipStopMonteCarlo
                        toolTipTextDisabled: Properties.tipSelectValidOutput
                        enabled: calculation.outputValid
                        onClicked: !Properties.outputLocked ?
                                       calculation.runMonteCarlo() :
                                       calculation.stopMonteCarlo()
                        KeyNavigation.backtab: addOutputParamButton
                        KeyNavigation.tab: addInputParamButton
                    }
                }

                ScrollView {
                    // ScrollView containing the Monte Carlo results table and
                    // the histogram

                    anchors {
                        top: monteCarloCard.header.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }
                    clip: false
                    ScrollBar.vertical: SigmaScrollBar {
                        anchors {
                            top: parent.top
                            bottom: parent.bottom
                            left: parent.right
                            leftMargin:
                                ( Properties.spacingM - indicatorWidth ) / 2
                        }
                    }
                    ScrollBar.horizontal: SigmaScrollBar {
                        anchors {
                            top: parent.bottom
                            topMargin:
                                ( Properties.spacingM - indicatorWidth ) / 2
                            left: parent.left
                            right: parent.right
                        }
                    }

                    Flickable {
                        // Reserve space for the histogram/busyindicator?
                        property bool extended: calculation.histogramValid ||
                                                monteCarloBusyIndicator.running

                        contentHeight: monteCarloResultsTable.height + (
                                       extended ? (
                                           Properties.spacingL +
                                           histogramHeading.height +
                                           Properties.spacingM +
                                           histogram.height +
                                           Properties.spacingM
                                        ) : 0 )
                        contentWidth: width
                        clip: true
                        boundsMovement: Flickable.StopAtBounds

                        MonteCarloResultsTable {
                            id: monteCarloResultsTable

                            onExtraRequested: { openCSV() }
                        }

                        Heading {
                            id: histogramHeading

                            visible: calculation.histogramValid
                            text: Properties.titleEstimatedDistribution
                            anchors {
                                top: monteCarloResultsTable.bottom
                                topMargin: Properties.spacingL
                                left: parent.left
                                right: parent.right
                            }
                        }

                        GraphsView {
                            id: histogram

                            anchors {
                                top: histogramHeading.bottom
                                topMargin: Properties.spacingM
                                left: parent.left
}
                            visible: !monteCarloBusyIndicator.running &&
                                     calculation.histogramValid
                            width: parent.width
                            height: width * 0.8
                            marginLeft: 0
                            marginRight: Properties.spacingM
                            marginTop: Properties.spacingXS
                            marginBottom: 0

                            axisX: ValueAxis {
                                property string title:
                                    Properties.outputName +
                                    " [" + Properties.outputUnit + "]"
                                titleText: Properties.outputParamAvailable ?
                                               title :
                                               ""
                                titleColor: Properties.colorTextWeak
                                min: calculation.histogramXMin
                                max: calculation.histogramXMax
                                tickInterval: (max - min) / 6
                                tickAnchor: (max - min) / 2
                                gridVisible: false
                                subGridVisible: false
                                labelFormat: "%.3g"
                                titleFont.family: window.font.family
                                titleFont.pixelSize: window.font.pixelSize
                            }

                            axisY: ValueAxis {
                                min: 0
                                max: calculation.histogramYMax
                                tickInterval: (max - min) / 5
                                gridVisible: false
                                subGridVisible: false
                                labelFormat: "%.3f"
                            }

                            BarSeries {
                                id: mySeries

                                // The 'active' property is used to show the
                                // animation on startup
                                property bool active: false

                                barWidth: 1
                                valuesMultiplier:
                                    histogram.visible && active ? 1 : 0
                                barDelegate: Item {
                                    property int barIndex
                                    property int l:
                                        calculation.histogramLowerIndex
                                    property int h:
                                        calculation.histogramHigherIndex
                                    property bool outside:
                                        barIndex < l || barIndex >= h
                                    Rectangle {
                                        color:
                                            outside ?
                                                Properties.colorStrokeStrong :
                                                Properties.colorBrand
                                        anchors {
                                            fill: parent
                                            bottomMargin: 1
                                        }
                                        antialiasing: true
                                    }
                                }

                                BarSet {
                                    id: histoValues

                                    values: calculation.histogramValues
                                }

                                Behavior on valuesMultiplier {
                                    NumberAnimation {
                                        duration: Properties.animationDuration
                                        easing.type: Easing.InOutQuad
                                    }
                                }

                                Component.onCompleted: active = true
                            }

                            theme: GraphsTheme {
                                    theme: GraphsTheme.Theme.UserDefined
                                    colorStyle: GraphsTheme.ColorStyle.Uniform
                                    backgroundVisible: false
                                    plotAreaBackgroundVisible: false

                                    axisX.labelTextColor:
                                        Properties.colorTextWeak
                                    axisX.mainColor:
                                        Properties.colorStrokeStrong
                                    axisX.mainWidth: Properties.borderWidth
                                    axisX.subColor: Properties.colorStrokeStrong
                                    axisXLabelFont.family: window.font.family
                                    axisXLabelFont.pixelSize:
                                        Properties.fontSizeHistogram

                                    // Set axisY properties equal to axisX
                                    axisY.labelTextColor: axisX.labelTextColor
                                    axisY.mainColor: axisX.mainColor
                                    axisY.mainWidth: axisX.mainWidth
                                    axisY.subColor: axisX.subColor
                                    axisYLabelFont.family: axisXLabelFont.family
                                    axisYLabelFont.pixelSize:
                                        axisXLabelFont.pixelSize
                                 }
                        }

                        SigmaBusyIndicator {
                            id: monteCarloBusyIndicator

                            anchors.centerIn: histogram
                            anchors.verticalCenterOffset: -Properties.spacingXL
                            running: Properties.outputLocked
                            visible: monteCarloBusyIndicator.running
                        }

                        ProgressBar {
                            id: monteCarloProgressBar

                            anchors {
                                left: parent.left
                                right: parent.right
                                top: histogram.bottom
                                topMargin: -Properties.spacingXL
                                margins: Properties.spacingL
                            }
                            height: Properties.progressBarHeight
                            value: calculation.monteCarloConvergenceFactor
                            visible: monteCarloBusyIndicator.running

                            background: Rectangle {
                                color: Properties.colorStrokeWeak
                                radius: height / 2
                            }

                            contentItem:  Rectangle {
                                anchors {
                                    left: parent.left
                                    right: parent.right
                                    rightMargin: parent.width *
                                                 (1 - parent.visualPosition )
                                    top: parent.top
                                    bottom: parent.bottom
                                }
                                height: parent.height
                                color: Properties.colorBrand
                                radius: height / 2
                            }
                        }
                    }
                }
            }
        }
    }

    // Instantiate the various dialogs
    InputParamWindow {
        id: inputParamWindow

        tableView: inputTable.tableView
    }

    OutputParamWindow {
        id: outputParamWindow

        tableView: outputTable.tableView
    }

    CorrelationsWindow {
        id: correlationsWindow

        tableView: inputTable.tableView
    }

    DeleteInputParameterDialog {
        id: deleteInputParamDialog

        transientParent: window
    }

    DeleteOutputParameterDialog {
        id: deleteOutputParamDialog

        transientParent: window
    }

    SettingsWindow {
        id: settingsWindow
    }

    AboutWindow {
        id: aboutWindow
    }

    ProjectDialog {
        id: fileDialog
    }

    ClearDialog {
        id: clearDialog

        transientParent: window
    }

    DiscardUnsavedChanges {
        id: discardUnsavedChanges

        transientParent: window
    }

    ProjectFailed {
        id: projectFailed

        transientParent: window
    }

    CSVDialog {
        id: csvDialog
    }
}
