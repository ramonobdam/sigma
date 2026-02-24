import QtQuick
import Sigma

// Dialog component to delete an input parameter
BaseDialog {
    id: control

    property string references

    function deleteAndHide() {
        // Remove the input parameter and hide the window
        calculation.removeInputParameter()
        hide()
    }

    onVisibleChanged: {
        if ( visible ) {
            Qt.callLater( centerOnParent )
            references = calculation.getSelectedInputParameterReferences()
            primaryButton.forceActiveFocus()
        }
    }

    title: "Delete input parameter"
    headerText: "Do you want to delete input parameter " +
                properties.inputName +
                "?"
    subText: references.length > 0 ?
                "It is detected in " + references :
                "It is currently not detected in any (valid) output parameter"

    primaryButton.text: properties.buttonTextDelete
    primaryButton.onClicked: { control.deleteAndHide() }
    primaryButton.KeyNavigation.tab: secondaryButton
    primaryButton.KeyNavigation.backtab: secondaryButton

    secondaryButton.KeyNavigation.tab: primaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.visible: false
}
