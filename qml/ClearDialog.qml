import QtQuick
import Sigma

// Dialog component to clear the project
BaseDialog {
    id: control

    function clearAndHide() {
        calculation.clearProject()
        hide()
    }

    title: "Clear project"

    headerText: "Do you want to clear the current project?"
    subText: "All parameters and simulation results will be deleted."

    primaryButton.text: "Clear project"
    primaryButton.onClicked: { control.clearAndHide() }
    primaryButton.KeyNavigation.tab: secondaryButton
    primaryButton.KeyNavigation.backtab: secondaryButton

    secondaryButton.KeyNavigation.tab: primaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.visible: false
}
