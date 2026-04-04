// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtCore
import Sigma

// Window with a form to change the persistent settings
BaseWindow {
    id: control

    // 'formActive' is used to make sure settings are not updated after a field
    // loses focus when the cancel button is clicked.
    property bool formActive

    // Do all form fields contain valid (acceptable) input?
    property bool allValid: displayPrecisionField.acceptableInput &&
                            csvPrecisionField.acceptableInput &&
                            monteCarloDigitsField.acceptableInput &&
                            monteCarloBatchSizeField.acceptableInput &&
                            monteCarloMaxBatchesField.acceptableInput

    function cancel() {
        control.formActive = false
        control.restoreInitialValues()
        control.hide()
    }

    function reset() {
        appSettings.setToDefaults()
        calculation.resetDisplay()
    }

    function save() {
        if ( control.allValid ) {
            // When Auto Save is switched on, save any unsaved changes
            if ( autoSaveCheckBox.checked && properties.unsavedChanges ) {
                calculation.saveProject()
            }
            control.hide();
        }
    }

    function storeInitialValues() {
        autoSaveCheckBox.storeInitial()
        restoreLastProjectCheckBox.storeInitial()
        radioButtonSystem.storeInitial()
        radioButtonLight.storeInitial()
        radioButtonDark.storeInitial()
        displayPrecisionField.storeInitial()
        csvPrecisionField.storeInitial()
        monteCarloDigitsField.storeInitial()
        monteCarloBatchSizeField.storeInitial()
        monteCarloMaxBatchesField.storeInitial()
    }

    function restoreInitialValues() {
        autoSaveCheckBox.restoreInitial()
        restoreLastProjectCheckBox.restoreInitial()
        radioButtonSystem.restoreInitial()
        radioButtonLight.restoreInitial()
        radioButtonDark.restoreInitial()
        displayPrecisionField.restoreInitial()
        csvPrecisionField.restoreInitial()
        monteCarloDigitsField.restoreInitial()
        monteCarloBatchSizeField.restoreInitial()
        monteCarloMaxBatchesField.restoreInitial()
    }

    windowTitle: "Settings"
    width: 380
    height: container.childrenRect.height +
            titleBar.height +
            container.anchors.margins * 2
    settingsKey: "settingsWindow"
    settingsObject: Settings {}

    onVisibleChanged: {
        if ( control.visible ) {
            // Make sure all input field have the currently stores values
            appSettings.notifyAllChanged()
            control.storeInitialValues()
            control.formActive = true
            saveButton.forceActiveFocus()
        }
    }

    Item {
        id: container
        anchors {
            left: parent.left
            right: parent.right
            top: titleBar.bottom
            bottom: parent.bottom
            margins: properties.spacingM        }

        focus: true
        Keys.onEscapePressed: { control.cancel() }

        FormSubHeading {
            id: displayThemeLabel

            anchors {
                topMargin: 0
            }

            text: "Color scheme"
        }

        SigmaRadioButton {
            id: radioButtonSystem

            property bool initial
            property int theme: DisplayTheme.System

            function storeInitial() {
                initial = checked
            }

            function restoreInitial() {
                if ( initial ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            anchors {
                top: displayThemeLabel.bottom
                topMargin: control.interLabelSpacing
                left: parent.left
            }

            text: "Follow system setting (" +
                  ( properties.systemThemeLight ? "light" : "dark") +
                  ")"
            checked: properties.displayTheme === theme
            onToggled: {
                if ( checked ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            KeyNavigation.backtab: resetButton
            KeyNavigation.tab: radioButtonLight
        }

        SigmaRadioButton {
            id: radioButtonLight

            property bool initial
            property int theme: DisplayTheme.Light

            function storeInitial() {
                initial = checked
            }

            function restoreInitial() {
                if ( initial ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            anchors {
                top: radioButtonSystem.bottom
                topMargin: control.interLabelSpacing
                left: parent.left
            }

            text: "Light"
            checked: properties.displayTheme === theme
            onToggled: {
                if ( checked ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            KeyNavigation.backtab: radioButtonSystem
            KeyNavigation.tab: radioButtonDark
        }

        SigmaRadioButton {
            id: radioButtonDark

            property bool initial
            property int theme: DisplayTheme.Dark

            function storeInitial() {
                initial = checked
            }

            function restoreInitial() {
                if ( initial ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            anchors {
                top: radioButtonLight.bottom
                topMargin: control.interLabelSpacing
                left: parent.left
            }

            text: "Dark"
            checked: properties.displayTheme === theme
            onToggled: {
                if ( checked ) {
                    appSettings.setDisplayTheme( theme )
                }
            }

            KeyNavigation.backtab: radioButtonLight
            KeyNavigation.tab: autoSaveCheckBox
        }

        FormSubHeading {
            id: projectLabel

            anchors {
                top: radioButtonDark.bottom
            }

            text: "Project"
        }

        SigmaCheckBox {
            id: autoSaveCheckBox

            property bool initial

            function storeInitial() {
                initial = checked
            }

            function restoreInitial() {
                appSettings.setAutoSaveProject( initial )
            }

            anchors {
                top: projectLabel.bottom
                topMargin: interLabelSpacing
                left: parent.left
            }

            checked: properties.autoSaveProject
            text: "Auto Save project"
            onToggled: { appSettings.setAutoSaveProject( checked ) }

            KeyNavigation.backtab: radioButtonDark
            KeyNavigation.tab: restoreLastProjectCheckBox
        }

        SigmaCheckBox {
            id: restoreLastProjectCheckBox

            property bool initial

            function storeInitial() {
                initial = checked
            }

            function restoreInitial() {
                appSettings.setRestoreLastProject( initial )
            }

            anchors {
                top: autoSaveCheckBox.bottom
                topMargin: control.interLabelSpacing
                left: parent.left
            }

            checked: properties.restoreLastProject
            text: "Restore last project on startup"
            onToggled: { appSettings.setRestoreLastProject( checked ) }

            KeyNavigation.backtab: autoSaveCheckBox
            KeyNavigation.tab: displayPrecisionField
        }

        FormSubHeading {
            id: numericalPrecisionLabel

            anchors {
                top: restoreLastProjectCheckBox.bottom
            }

            text: "Significant numerical digits"
            subText: "Interface (" +
                     displayPrecisionField.from +
                     " to " +
                     displayPrecisionField.to +
                     ")"
        }

        Stepper {
            id: displayPrecisionField

            property int initial

            function storeInitial() {
                initial = value
            }
            function restoreInitial() {
                appSettings.setDisplayPrecision( initial )
                calculation.resetDisplay()
            }

            anchors {
                top: numericalPrecisionLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            from: 1
            to: 10
            value: appSettings.displayPrecision
            onValueModified: {
                if ( control.formActive ) {
                    appSettings.setDisplayPrecision( value )
                    calculation.resetDisplay()
                }
            }

            KeyNavigation.backtab: restoreLastProjectCheckBox
            KeyNavigation.tab: csvPrecisionField
        }

        SigmaText {
            id: csvPrecisionLabel

            anchors {
                top: displayPrecisionField.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "CSV export (" +
                  csvPrecisionField.from +
                  " to " +
                  csvPrecisionField.to +
                  ")"
        }

        Stepper {
            id: csvPrecisionField

            property int initial

            function storeInitial() {
                initial = value
            }

            function restoreInitial() {
                appSettings.setCSVPrecision( initial )
            }

            anchors {
                top: csvPrecisionLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            from: 1
            to: 20
            value: appSettings.csvPrecision
            onValueModified: {
                if ( formActive ) {
                    appSettings.setCSVPrecision( value )
                }
            }

            KeyNavigation.backtab: displayPrecisionField
            KeyNavigation.tab: monteCarloDigitsField
        }

        FormSubHeading {
            id: monteCarloLabel

            anchors {
                top: csvPrecisionField .bottom
            }

            text: "Monte Carlo simulation"
            subText: "Convergence significant digits (" +
                     monteCarloDigitsField.from +
                     " to " +
                     monteCarloDigitsField.to +
                     ")"
        }

        Stepper {
            id: monteCarloDigitsField

            property int initial

            function storeInitial() {
                initial = value
            }

            function restoreInitial() {
                appSettings.setMonteCarloDigits( initial )
            }

            anchors {
                top: monteCarloLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            from: 1
            to: 3
            value: appSettings.monteCarloDigits
            onValueModified: {
                if ( control.formActive ) {
                    appSettings.setMonteCarloDigits( value )
                }
            }

            KeyNavigation.backtab: csvPrecisionField
            KeyNavigation.tab: monteCarloBatchSizeField
        }

        SigmaText {
            id: monteCarloBatchSizeLabel

            anchors {
                top: monteCarloDigitsField.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Batch size (" +
                  monteCarloBatchSizeField.fromString +
                  " to " +
                  monteCarloBatchSizeField.toString +
                  ")"
        }

        StepperExp {
            id: monteCarloBatchSizeField

            property int initial

            function storeInitial() {
                initial = number
            }

            function restoreInitial() {
                appSettings.setMonteCarloBatchSize( initial )
            }

            anchors {
                top: monteCarloBatchSizeLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            numberFrom: 1e2
            numberTo: 1e6
            value: numberToValue( appSettings.monteCarloBatchSize )
            onValueModified: {
                if ( control.formActive) {
                    appSettings.setMonteCarloBatchSize( number )
                }
            }

            KeyNavigation.backtab: monteCarloDigitsField
            KeyNavigation.tab: monteCarloMaxBatchesField
        }

        SigmaText {
            id: monteCarloMaxBatchesLabel

            anchors {
                top: monteCarloBatchSizeField.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Maximum number of batches (" +
                  monteCarloMaxBatchesField.fromString +
                  " to " +
                  monteCarloMaxBatchesField.toString +
                  ")"
        }

        StepperExp {
            id: monteCarloMaxBatchesField

            property int initial

            function storeInitial() {
                initial = number
            }
            function restoreInitial() {
                appSettings.setMonteCarloMaxOfNumBatches( initial )
            }

            anchors {
                top: monteCarloMaxBatchesLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            numberFrom: 1e1
            numberTo: 1e5
            value: numberToValue( appSettings.monteCarloMaxNumOfBatches )
            onValueModified: {
                if ( control.formActive ) {
                    appSettings.setMonteCarloMaxOfNumBatches( number )
                }
            }

            KeyNavigation.backtab: monteCarloBatchSizeField
            KeyNavigation.tab: saveButton
        }

        FormSubHeading {
            id: horizontalRule

            anchors {
                top: monteCarloMaxBatchesField.bottom
            }
        }

        PrimaryButton {
            id: saveButton

            anchors {
                top: horizontalRule.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Save settings"
            onClicked: { control.save() }

            KeyNavigation.backtab: monteCarloMaxBatchesField
            KeyNavigation.tab: cancelButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        SecondaryButton {
            id: cancelButton

            anchors {
                top: saveButton.top
                left: saveButton.right
                leftMargin: properties.spacingButtons
            }

            text: properties.buttonTextCancel
            onClicked: { control.cancel() }

            KeyNavigation.backtab: saveButton
            KeyNavigation.tab: resetButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        SecondaryButton {
            id: resetButton

            anchors {
                top: saveButton.top
                right: parent.right
            }

            text: "Reset all"
            onClicked: { control.reset() }

            KeyNavigation.backtab: cancelButton
            KeyNavigation.tab: radioButtonSystem
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }
    }
}
