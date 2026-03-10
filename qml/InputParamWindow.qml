// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtCore
import QtQuick.Controls
import Sigma

// Window component to add a new input parameter or to edit an existing one
BaseWindow {
    id: control

    property InputParameter param: calculation.getQMLInputParameter()
    property bool edit: true
    property bool allAcceptableInputs: nameField.valid &&
                                       nominalField.acceptableInput &&
                                       stdUncertaintyField.acceptableInput &&
                                       (
                                            dofStepper.acceptableInput ||
                                            dofInfinite
                                       )
    property bool noneDistribution: distribution === "none"
    property TableView tableView
    property alias name: nameField.text
    property alias unit: unitCombo.editText
    property alias nominal: nominalField.text
    property alias stdUncertainty: stdUncertaintyField.text
    property alias distribution: distributionCombo.editText
    property alias dofInfinite: dofCheckBox.checked
    property alias dof: dofStepper.value

    function open( editMode = true ) {
        // Show the window and set editMode (i.e. edit an existing parameter or
        // add a new one)
        control.edit = editMode
        control.show()
    }

    function getParam() {
        // Set the input field values to the selected parameter in the
        // TableView
        calculation.setQMLInputParameterToSelected()
        control.name = param.getName()
        control.unit = param.getUnit()
        control.getCurrentIndexUnit()
        control.nominal = param.getNominalValue()
        control.stdUncertainty = param.getStdUncertainty()
        control.distribution = param.getDistributionAsString()
        control.getCurrentIndexDistribution()
        control.dofInfinite = param.getDOFInfinite()
        control.dof = param.getDOF()
    }

    function setParam() {
        // Set param to the current input field values
        control.param.setName( control.name )
        control.param.setUnit( control.unit )
        control.param.setNominalValue( control.nominal )

        // stdUncertainty is set to zero when the 'none' distribution type is
        // chosen
        let std = control.noneDistribution ? 0 : control.stdUncertainty

        control.param.setStdUncertainty( std )
        control.param.setDistribution( control.distribution )
        control.param.setDOFInfinite( control.dofInfinite )
        control.param.setDOF( control.dof )
    }

    function updateParam() {
        // Update the selected parameter in the model to the current input field
        // values
        control.setParam()
        calculation.updateInputParameter(param)
    }

    function addParam() {
        // Add a new InputParameter
        control.setParam()
        calculation.addInputParameter( param )
    }

    function addUnit() {
        // Add a new unit to the model (if unique)
        if ( unitCombo.find( control.unit ) === -1 ) {
            let newUnit = control.unit
            calculation.addUnit( newUnit )
            control.unit = newUnit
            control.getCurrentIndexUnit()
        }
    }

    function getCurrentIndexUnit() {
        // Update the currentIndex of the ComboBox to the current unit
        unitCombo.currentIndex = unitCombo.indexOfValue( control.unit )
    }

    function getCurrentIndexDistribution() {
        // Update the currentIndex of the ComboBox to the current distribution
        distributionCombo.currentIndex = distributionCombo.indexOfValue(
                                             control.distribution
                                         )
    }

    function saveAndReturn( currentColumn = 0 ) {
        // Save the parameter (when it is valid), close the popup and return
        // focus to TableView
        if ( control.allAcceptableInputs ) {
            control.addUnit()
            if ( control.edit ) {
                control.updateParam()
            }
            else {
                control.addParam()
            }
            control.close()
            control.returnFocus( currentColumn )
        }
    }

    function returnFocus( currentColumn = 0 ) {
        // Return the active focus to the TableView and set the current column
        // to the active input field when in edit mode
        if ( control.edit ) {
            let currentRow = control.tableView.currentRow
            let index = control.tableView.index( currentRow, currentColumn )
            control.tableView.selectionModel.setCurrentIndex(
                                                 index,
                                                 ItemSelectionModel.Rows
                                             )
        }
    }

    function setActiveFocus() {
        // Set activeFocus to current column of the table view when in editing
        // mode or to name field when adding a new parameter (!edit)
        let focusReason = Qt.PopupFocusReason
        if ( control.edit ) {
            switch ( control.tableView.currentColumn ) {
                case 1:
                    unitCombo.forceActiveFocus( focusReason )
                    break
                case 2:
                    nominalField.forceActiveFocus( focusReason )
                    break
                case 3:
                    stdUncertaintyField.forceActiveFocus( focusReason )
                    break
                case 4:
                    distributionCombo.forceActiveFocus( focusReason )
                    break
                case 5:
                    dofCheckBox.forceActiveFocus( focusReason )
                    break
                default:
                    nameField.forceActiveFocus( focusReason )
            }
        }
        else {
            nameField.forceActiveFocus( focusReason )
        }
    }

    function showValidName() {
        nameField.valid = calculation.validInputName(
                                          control.name,
                                          control.edit
                                      )
    }

    width: 385
    height: container.childrenRect.height +
            titleBar.height +
            container.anchors.margins * 2
    windowTitle: ( control.edit ? "Edit" : "Add new" ) + " input parameter"
    settingsKey: "inputParamWindow"
    settingsObject: Settings {}
    onVisibleChanged: {
        if ( control.visible ) {
            control.restorePosition()
            control.getParam()
            if ( !control.edit ) {
                control.name = calculation.getNewInputParameterName()
            }
            control.setActiveFocus()
            control.showValidName()
        }
    }

    SigmaProperties {
        id: properties
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
        Keys.onEscapePressed: { control.close() }

        FormSubHeading {
            id: nameLabel

            anchors {
                topMargin: 0
            }

            text: "Input name"
            subText: "A unique name that starts with a letter"
        }

        SymbolTextField {
            id: nameField

            anchors {
                top: nameLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            maximumLength: 50
            unacceptableInputMessage: "Enter a valid input name"

            textField.onTextEdited: { control.showValidName() }
            textField.onAccepted: { control.saveAndReturn( 0 ) }

            KeyNavigation.backtab: cancelButton
            KeyNavigation.tab: unitCombo
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        FormSubHeading {
            id: unitLabel

            anchors {
                top: nameField.bottom
            }

            text: "Unit"
            subText: "Unit of the input estimate and standard uncertainty"
        }

        SymbolComboBox {
            id: unitCombo

            anchors {
                top: unitLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            model: calculation.unitsModel()
            editable: true
            maximumLength: 20
            maximumHeightPopup: horizontalRule.y - y - height
            comboBox.onAccepted: { control.saveAndReturn( 1 ) }

            KeyNavigation.backtab: nameField
            KeyNavigation.tab: nominalField
        }

        FormSubHeading {
            id: nominalLabel

            anchors {
                top: unitCombo.bottom
            }

            text: "Input estimate [" + control.unit + "]"
            subText: "Nominal value of the input parameter"
        }

        SigmaTextField {
            id: nominalField

            anchors {
                top: nominalLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            validator: DoubleValidator {
                notation: DoubleValidator.ScientificNotation
            }

            onAccepted: { control.saveAndReturn( 2 ) }

            KeyNavigation.backtab: unitCombo
            KeyNavigation.tab: stdUncertaintyField
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        FormSubHeading {
            id: stdUncertaintyLabel

            anchors {
                top: nominalField.bottom
            }

            text: "Standard uncertainty [" + control.unit + "]"
            subText: "Estimated standard deviation of the input parameter (≥0)"
        }

        SigmaTextField {
            id: stdUncertaintyField

            anchors {
                top: stdUncertaintyLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            // Disable the field and its validator when the probability
            // distribution is none. The standard uncertainty is not used in
            // this situation and set to 0 for clarity.
            enabled: !control.noneDistribution
            validator: enabled ? doubleValidator : null

            onAccepted: { control.saveAndReturn( 3 ) }

            KeyNavigation.backtab: nominalField
            KeyNavigation.tab: distributionCombo
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab

            DoubleValidator {
                id: doubleValidator
                bottom: 0
                notation: DoubleValidator.ScientificNotation
            }

            SigmaToolTip {
                visible: stdUncertaintyField.hovered &&
                         !stdUncertaintyField.enabled
                text: properties.tipNoDistribution
            }
        }

        FormSubHeading {
            id: distributionLabel

            anchors {
                top: stdUncertaintyField.bottom
            }

            text: "Probability distribution type"
            subText: "The shape of the probability density function"
        }

        ComboBoxListModel {
            id: distributionCombo

            anchors {
                top: distributionLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            model: calculation.distributionsModel()
            editable: false
            onAccepted: { control.saveAndReturn( 4 ) }

            KeyNavigation.backtab: stdUncertaintyField
            KeyNavigation.tab: dofCheckBox
        }

        FormSubHeading {
            id: dofLabel

            anchors {
                top: distributionCombo.bottom
            }

            text: "Degrees of freedom"
            subText: "Infinite or a specified value (" +
                     dofStepper.from +
                     " to " +
                     Number( dofStepper.to ).toLocaleString( locale, 'e', 0 ) +
                     ")"
        }

        SigmaCheckBox {
            id: dofCheckBox

            anchors {
                top: dofLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }

            text: "Infinite"

            KeyNavigation.backtab: distributionCombo
            KeyNavigation.tab: dofStepper
        }

        Stepper {
            id: dofStepper

            anchors {
                top: dofCheckBox.top
                left: dofCheckBox.right
                leftMargin: properties.spacingL
            }
            inputFieldWidth: 50

            visible: !control.dofInfinite

            from: 1
            to: 1e6

            validator: DoubleValidator {
                bottom: dofStepper.from
                top: dofStepper.to
                decimals: 0
                notation: DoubleValidator.ScientificNotation
            }

            KeyNavigation.backtab: dofCheckBox
            KeyNavigation.tab: saveButton
        }


        FormSubHeading {
            id: horizontalRule
            anchors {
                top: dofCheckBox.bottom
                topMargin: control.topLabelSpacing
            }
        }


        PrimaryButton {
            id: saveButton

            anchors {
                top: horizontalRule.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Save input parameter"

            onClicked: {
                // Set the active focus to this button to make a pending input
                // field is evaluated.
                saveButton.forceActiveFocus()
                control.saveAndReturn()
            }

            KeyNavigation.backtab: dofStepper
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

            onClicked: { control.close() }

            KeyNavigation.backtab: saveButton
            KeyNavigation.tab: nameField
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }
    }
}
