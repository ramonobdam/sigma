import QtQuick
import Sigma

// Dialog component to delete an output parameter
BaseDialog {
    id: control

    function deleteAndHide() {
        // Remove the output parameter and hide the window
        calculation.removeOutputParameter()
        hide()
    }

    title: "Delete output parameter"
    headerText: "Do you want to delete output parameter " +
                properties.outputName +
                "?"
    subText: "Results for this output parameter will be lost"

    primaryButton.text: properties.buttonTextDelete
    primaryButton.onClicked: { control.deleteAndHide() }
    primaryButton.KeyNavigation.tab: secondaryButton
    primaryButton.KeyNavigation.backtab: secondaryButton

    secondaryButton.KeyNavigation.tab: primaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.visible: false
}
