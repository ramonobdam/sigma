// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtCore
import QtQuick.Controls
import Sigma

// Window component to add a new output parameter or to edit an existing one
BaseWindow {
    id: control

    property bool edit: true
    property bool valid
    property bool allAcceptableInputs: nameField.valid &&
                                       confidenceField.acceptableInput
    property string nominal
    property string error
    property TableView tableView
    property alias name: nameField.text
    property alias unit: unitCombo.editText
    property alias formula: formulaField.text
    property alias insertParam: paramCombo.editText
    property alias confidence: confidenceField.text

    function open( editMode = true ) {
        // Show the window and set editMode (i.e. edit an existing parameter or
        // add a new one)
        control.edit = editMode
        control.show()
    }

    function getParam() {
        // Set the input field values to the selected parameter in the TableView
        param.setToSelected();
        control.name = param.getName()
        control.unit = param.getUnit()
        control.getCurrentIndexUnit()
        control.formula = param.getFormula()
        control.nominal = param.getNominalValueAsString()
        control.error = param.getError()
        control.valid = param.getValid()
        control.confidence =
                Number( param.getConfidence() * 100. ).toLocaleString()
    }

    function setParam() {
        // Set param to the current input field values
        param.setName( control.name )
        param.setUnit( control.unit )
        param.setFormula( control.formula )
        param.setConfidence(
                          Number.fromLocaleString( control.confidence ) / 100.
                      )
    }

    function updateParam() {
        // Update the selected parameter in the model to the current input field
        // values
        control.setParam()
        calculation.updateOutputParameter( param )
    }

    function addParam() {
        // Add a new OutputParameter
        param.setLocked( false );
        control.setParam()
        calculation.addOutputParameter( param )
    }

    function addUnit() {
        // Add a new unit to the model (if unique)
        if ( unitCombo.find( control.unit ) === -1) {
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

    function setCurrentIndexParam() {
        // Set the currentIndex to the first item is none is selected
        if ( paramCombo.editText.length === 0 ) {
            paramCombo.currentIndex = 0
        }
    }

    function saveAndReturn( currentColumn = 0 ) {
        // Save the parameter (when the name is valid), close the popup and
        // return focus to TableView
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
            switch( control.tableView.currentColumn ) {
                case 1:
                    formulaField.forceActiveFocus( focusReason )
                    break
                case 2:
                    unitCombo.forceActiveFocus( focusReason )
                    break
                case 3:
                    confidenceField.forceActiveFocus( focusReason )
                    break;
                default:
                    nameField.forceActiveFocus( focusReason )
            }
        }
        else {
            nameField.forceActiveFocus( focusReason )
        }
    }

    function showValidName() {
        nameField.valid = calculation.validOutputName(
                              control.name,
                              control.edit
                          )
    }

    function evaluate() {
        // Evaluate the current measurement function and set the nominal value
        // and error
        control.setParam()
        param.compile()
        control.nominal = param.getNominalValueAsString()
        control.valid = param.getValid()
        control.error = param.getError()
    }

    width: 528
    height: container.childrenRect.height +
            titleBar.height +
            container.anchors.margins * 2
    windowTitle: ( control.edit ? "Edit" : "Add new" ) + " output parameter"
    settingsKey: "outputParamWindow"
    settingsObject: Settings {}
    onVisibleChanged: {
        if ( control.visible ) {
            control.getParam()
            if ( !control.edit ) {
                control.name = calculation.getNewOutputParameterName()
                control.formula = ""
            }
            control.evaluate()
            control.setActiveFocus()
            control.showValidName()
            control.setCurrentIndexParam()
        }
    }

    OutputParameter {
        id: param
    }

    SigmaFonts {
        id: fonts
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

            text: "Output name"
            subText: "A unique name of at least one character"
        }

        SymbolTextField {
            id: nameField

            anchors {
                top: nameLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            unacceptableInputMessage: "Enter a valid output name"
            maximumLength: 50
            textField.onTextEdited: { control.showValidName() }
            textField.onAccepted: { control.saveAndReturn( 0 ) }

            KeyNavigation.backtab: cancelButton
            KeyNavigation.tab: formulaField
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        FormSubHeading {
            id: formulaLabel

            anchors {
                top: nameField.bottom
            }

            text: "Measurement function"
            subText: "The relation between the output parameter and the input" +
                     " parameters"
        }

        SymbolTextField {
            id: formulaField

            anchors {
                top: formulaLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: 398

            buttonIcon: properties.calculatorIcon
            buttonFont.family: fonts.fontAwesome.font.family
            buttonFont.pixelSize: properties.fontSizeComboBoxIcon
            maximumLength: 1000
            textField.onAccepted: { control.saveAndReturn( 1 ) }
            buttonList: ListModel {
                ListElement { buttonText: "+"; symbol: "+" }
                ListElement { buttonText: "-"; symbol: "-" }
                ListElement { buttonText: "*"; symbol: "*" }
                ListElement { buttonText: "/"; symbol: "/" }
                ListElement { buttonText: "%"; symbol: "%" }
                ListElement { buttonText: "x²"; symbol: "^2" }
                ListElement { buttonText: "√"; symbol: "sqrt()" }
                ListElement { buttonText: "⌃"; symbol: "^" }
                ListElement { buttonText: "e"; symbol: "exp()" }
                ListElement { buttonText: "π"; symbol: "pi" }
                ListElement { buttonText: "("; symbol: "(" }
                ListElement { buttonText: ")"; symbol: ")" }
                ListElement { buttonText: "sin()"; symbol: "sin()" }
                ListElement { buttonText: "cos()"; symbol: "cos()" }
                ListElement { buttonText: "tan()"; symbol: "tan()" }
                ListElement { buttonText: "asin()"; symbol: "asin()" }
                ListElement { buttonText: "acos()"; symbol: "acos()" }
                ListElement { buttonText: "atan()"; symbol: "atan()" }
                ListElement { buttonText: "abs()"; symbol: "abs()" }
                ListElement { buttonText: "sgn()"; symbol: "sgn()" }
                ListElement { buttonText: "avg()"; symbol: "avg()" }
                ListElement { buttonText: "sum()"; symbol: "sum()" }
                ListElement { buttonText: "min()"; symbol: "min()" }
                ListElement { buttonText: "max()"; symbol: "max()" }
                ListElement { buttonText: "mul()"; symbol: "mul()" }
                ListElement { buttonText: "log()"; symbol: "log()" }
                ListElement { buttonText: "log₁₀()"; symbol: "log10()" }
                ListElement { buttonText: "deg2rad()"; symbol: "deg2rad()" }
                ListElement { buttonText: "rad2deg()"; symbol: "rad2deg()" }
            }
            rows: 4
            columns: 12

            KeyNavigation.tab: evaluateButton
            KeyNavigation.backtab: nameField
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        SecondaryButton {
            id: evaluateButton

            anchors {
                verticalCenter: formulaField.verticalCenter
                left: formulaField.right
                leftMargin: properties.spacingButtons
            }

            text: "Evaluate"
            onClicked: { control.evaluate() }

            KeyNavigation.backtab: formulaField
            KeyNavigation.tab: paramCombo
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        Item {
            id: statusMessage

            anchors {
                top: formulaField.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
                right: parent.right
            }
            height: errorText.height

            visible: control.error.length > 0

            Rectangle {
                id: icon

                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                }
                width: height

                radius: height / 2
                color: properties.colorTransparent
                border.width: properties.borderWidth
                border.color: control.valid ? properties.colorSuccess500 :
                                              properties.colorError500

                Text {
                    anchors.fill: parent

                    text: control.valid ? properties.validIcon :
                                          properties.invalidIcon
                    font.family: fonts.fontAwesome.font.family
                    font.pixelSize: properties.fontSizeUnacceptableInputIcon
                    color: icon.border.color
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            SigmaText {
                id: errorText

                anchors {
                    top: icon.top
                    left: icon.right
                    leftMargin: properties.spacingXS
                    right: parent.right
                }

                text: control.error
                clip: true
                elide: Text.ElideRight
            }
        }

        SigmaText {
            id: insertParamLabel

            anchors {
                top: statusMessage.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Insert input parameter"
        }

        ComboBoxListModel {
            id: paramCombo

            anchors {
                top: insertParamLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            model: calculation.inputItemModel()
            currentIndex: 0
            enabled: count > 0
            maximumHeightPopup: horizontalRule.y - y
            onAccepted: { formulaField.insertAtCursor( insertParam ) }

            // Do not accept focus when clicked. This makes sure the text
            // selection in the formulaField is not lost when using the mouse.
            focusPolicy: Qt.TabFocus

            KeyNavigation.backtab: evaluateButton
            KeyNavigation.tab: insertButton
        }

        SecondaryButton {
            id: insertButton

            anchors {
                verticalCenter: paramCombo.verticalCenter
                left: paramCombo.right
                leftMargin: properties.spacingButtons
            }

            text: "Insert"
            enabled: paramCombo.count > 0
            onClicked: { formulaField.insertAtCursor( insertParam ) }

            KeyNavigation.backtab: paramCombo
            KeyNavigation.tab: unitCombo
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        FormSubHeading {
            id: unitLabel

            anchors {
                top: paramCombo.bottom
            }

            text: "Unit"
            subText: "The unit of the output value"
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
            comboBox.onAccepted: { control.saveAndReturn( 2 ) }

            KeyNavigation.backtab: paramCombo
            KeyNavigation.tab: confidenceField
        }

        FormSubHeading {
            id: confidenceLabel

            anchors {
                top: unitCombo.bottom
            }

            text: "Level of confidence [%]"
            subText: "Required level of confidence of the expanded " +
                     "uncertainty (" +
                     confidenceField.bottomString +
                     "% to " +
                     confidenceField.topString +
                     "%)"
        }

        SigmaTextField {
            id: confidenceField

            property real validatorBottom: 0
            property real validatorTop: 99.9
            property int decimals: 1
            property string bottomString:
                Number( validatorBottom ).toLocaleString( locale, 'f', decimals)
            property string topString:
                Number( validatorTop ).toLocaleString( locale, 'f', decimals )

            anchors {
                top: confidenceLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            validator: DoubleValidator {
                            notation: DoubleValidator.StandardNotation
                            decimals: confidenceField.decimals
                            bottom: confidenceField.validatorBottom
                            top: confidenceField.validatorTop
                       }

            onAccepted: { control.saveAndReturn( 3 ) }

            KeyNavigation.backtab: unitCombo
            KeyNavigation.tab: saveButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        FormSubHeading {
            id: horizontalRule

            anchors {
                top: confidenceField.bottom
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

            text: "Save output parameter"
            onClicked: { control.saveAndReturn() }

            KeyNavigation.backtab: confidenceField
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
