import QtQuick
import QtCore
import QtQuick.Controls
import Sigma

// Window component to add a new correlation or to edit an existing one
BaseWindow {
    id: control

    property Correlation correlation: calculation.getQMLCorrelation()
    property bool edit: true
    property bool allAcceptableInputs: paramACombo.valid &&
                                       paramBCombo.valid &&
                                       coefField.acceptableInput &&
                                       valid
    property bool valid
    property string unacceptableInputMessage
    property TableView tableView
    property alias paramA: paramACombo.editText
    property alias paramB: paramBCombo.editText
    property alias coef: coefField.text

    function open( editMode = true ) {
        control.edit = editMode
        control.show()
    }

    function getCorrelation() {
        // Set the input field values to the selected correlation
        calculation.setQMLCorrelationToSelected()
        control.getCurrentIndexParamA( correlation.getInputParameterNameA() )
        control.getCurrentIndexParamB( correlation.getInputParameterNameB() )
        control.coef = correlation.getCorrelation()
    }

    function setCorrelation() {
        // Set correlation to the current input field values
        control.correlation.setInputParameterA( control.paramA )
        control.correlation.setInputParameterB( control.paramB )
        control.correlation.setCorrelation( control.coef )
    }

    function updateCorrelation() {
        // Update the selected correlation in the model to the current input
        // field values
        control.setCorrelation()
        calculation.updateCorrelation( control.correlation )
    }

    function addCorrelation() {
        // Add a new Correlation
        control.setCorrelation()
        calculation.addCorrelation( control.correlation )
    }

    function saveAndReturn( currentColumn = 0 ) {
        // Save the correlataion (when it is valid), close the popup and return
        // focus to TableView
        if ( control.allAcceptableInputs ) {
            if ( control.edit ) {
                control.updateCorrelation()
            }
            else {
                control.addCorrelation()
            }
            control.close()
            control.returnFocus( currentColumn )
        }
    }

    function returnFocus( currentColumn = 0 ) {
        // Return the active focus to the TableView and set the current column
        // to the active input field when in edit mode
        if ( control.edit && control.tableView ) {
            let currentRow = control.tableView.currentRow
            let index = control.tableView.index( currentRow, currentColumn )
            control.tableView.selectionModel.setCurrentIndex(
                                                 index,
                                                 ItemSelectionModel.Rows
                                             )
        }
    }

    function setActiveFocus() {
        // Set activeFocus to current column of the TableView when in editing
        // mode or to parameter A field when adding a new correlation (!edit)
        let focusReason = Qt.PopupFocusReason
        if ( control.edit && control.tableView ) {
            switch( control.tableView.currentColumn ) {
                case 1:
                    paramBCombo.forceActiveFocus( focusReason )
                    break
                case 2:
                    coefField.forceActiveFocus( focusReason )
                    break
                default:
                    paramACombo.forceActiveFocus( focusReason )
            }
        }
        else {
            paramACombo.forceActiveFocus( focusReason )
        }
    }

    function showValid() {
        // Check the validity of the proposed correlation
        // The parameters can't be constants
        paramACombo.valid = !calculation.inputParameterIsConstant(
                                             control.paramA
                                         )
        paramBCombo.valid = !calculation.inputParameterIsConstant(
                                             control.paramB
                                         )

        // Both parameters need to be different
        if ( control.paramA == control.paramB ) {
            control.valid = false
            control.unacceptableInputMessage =
                    properties.errorIdenticalInputParam
            return
        }

        // The correlation must be unique or equal to the correlation being
        // being edited
        if ( !control.correlation.isUnique( control.edit ) ) {
            control.valid = false
            control.unacceptableInputMessage =
                    properties.errorCorrelationNotUnique
            return
        }

        control.valid = true
        control.unacceptableInputMessage = ""
    }

    function getCurrentIndexParamA( value ) {
        // Update the currentIndex of the ComboBox based on value
        let index = paramACombo.indexOfValue( value )
        index = index < 0 ? 0 : index   // select first element when not found
        paramACombo.currentIndex = index
    }

    function getCurrentIndexParamB( value ) {
        // Update the currentIndex of the ComboBox based on value
        let index = paramBCombo.indexOfValue( value )
        index = index < 0 ? 1 : index   // select second element when not found
        paramBCombo.currentIndex = index
    }

    width: 440
    height: container.childrenRect.height +
            titleBar.height +
            container.anchors.margins * 2
    windowTitle: ( control.edit ? "Edit" : "Add new" ) + " correlation"
    settingsKey: "correlationEditWindow"
    settingsObject: Settings {}
    onVisibleChanged: {
        if ( control.visible ) {
            control.getCorrelation()
            control.setActiveFocus()
            control.showValid()
        }
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
            id: paramALabel

            anchors {
                topMargin: 0
            }

            text: "Input parameter A"
            subText: "Select the first input parameter"
        }

        ComboBoxListModel {
            id: paramACombo

            anchors {
                top: paramALabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            model: calculation.inputItemModel()
            enabled: count > 0
            maximumHeightPopup: horizontalRule.y - y
            onAccepted: { saveAndReturn( 0 ) }
            onActivated: { setCorrelation(); showValid() }
            unacceptableInputMessage: properties.errorConstantParamCor

            KeyNavigation.backtab: cancelButton
            KeyNavigation.tab: paramBCombo
        }


        FormSubHeading {
            id: paramBLabel

            anchors {
                top: paramACombo.bottom
            }

            text: "Input parameter B"
            subText: "Select the second input parameter"
        }

        ComboBoxListModel {
            id: paramBCombo

            anchors {
                top: paramBLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            model: calculation.inputItemModel()
            enabled: count > 0
            maximumHeightPopup: horizontalRule.y - y
            onAccepted: { saveAndReturn( 1 ) }
            onActivated: { setCorrelation(); showValid() }
            unacceptableInputMessage: properties.errorConstantParamCor

            KeyNavigation.backtab: paramACombo
            KeyNavigation.tab: coefField
        }

        FormSubHeading {
            id: coefLabel

            anchors {
                top: paramBCombo.bottom
            }

            text: "Correlation [-]"
            subText: "Correlation coefficient (" +
                     coefField.bottomString +
                     " to " +
                     coefField.topString +
                     ")"
        }

        SigmaTextField {
            id: coefField

            property real bottomValue: -1
            property real topValue: 1
            property string bottomString:
                Number( bottomValue ).toLocaleString( locale, "f", 0)
            property string topString:
                Number( topValue ).toLocaleString( locale, "f", 0)

            anchors {
                top: coefLabel.bottom
                topMargin: control.bottomLabelSpacing
                left: parent.left
            }
            width: properties.fieldWidth

            validator: DoubleValidator {
                notation: DoubleValidator.ScientificNotation
                bottom: coefField.bottomValue
                top: coefField.topValue
            }
            onAccepted: { saveAndReturn( 2 ) }

            KeyNavigation.backtab: paramBCombo
            KeyNavigation.tab: saveButton
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }

        Item {
            id: unAcceptableCorrelation

            anchors {
                top: coefField.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }
            height: icon.height
            visible: !control.valid

            Rectangle {
                id: icon

                anchors {
                    top: parent.top
                    left: parent.left
                }

                height: errorText.height
                width: height
                radius: height / 2
                color: properties.colorTransparent
                border.width: properties.borderWidth
                border.color: properties.colorError500

                Text {
                    anchors.fill: parent

                    text: properties.invalidIcon
                    font.family: fonts.fontAwesome.font.family
                    font.pixelSize: properties.fontSizeUnacceptableInputIcon
                    color: icon.border.color
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Text {
                id: errorText

                anchors {
                    top: icon.top
                    left: icon.right
                    leftMargin: properties.spacingXS
                }

                text: control.unacceptableInputMessage
                color: icon.border.color
            }
        }


        FormSubHeading {
            id: horizontalRule

            anchors {
                top: unAcceptableCorrelation.bottom
                topMargin: control.interFieldSpacing
            }
        }


        PrimaryButton {
            id: saveButton

            anchors {
                top: horizontalRule.bottom
                topMargin: control.interFieldSpacing
                left: parent.left
            }

            text: "Save correlation"
            onClicked: {
                // Set the active focus to this button to make sure a pending
                // input field is evaluated
                saveButton.forceActiveFocus()
                control.saveAndReturn()
            }

            KeyNavigation.backtab: coefField
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
            KeyNavigation.tab: paramACombo
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
        }
    }
}
