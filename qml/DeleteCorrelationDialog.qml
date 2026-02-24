import QtQuick
import Sigma

// Dialog component to delete a correlation
BaseDialog {
    id: control

    property string references
    property string paramA
    property string paramB
    property Correlation correlation: calculation.getQMLCorrelation()

    function deleteAndHide() {
        // Remove the correlation and hide the window
        calculation.removeCorrelation()
        hide()
    }

    onVisibleChanged: {
        if ( visible ) {
            Qt.callLater( centerOnParent )
            calculation.setQMLCorrelationToSelected()
            paramA = correlation.getInputParameterNameA()
            paramB = correlation.getInputParameterNameB()
            references = calculation.getSelectedCorrelationReferences()
            primaryButton.forceActiveFocus()
        }
    }

    title: "Delete correlation"
    headerText:
        "Do you want to delete the correlation between input parameters " +
        paramA +
        " and " +
        paramB +
        "?"
    subText: references.length > 0 ?
                 "The correlation is detected in " + references :
                 (
                     "The correlation is currently not detected in any (valid) "
                   + "output parameter"
                 )

    primaryButton.text: properties.buttonTextDelete
    primaryButton.onClicked: { control.deleteAndHide() }
    primaryButton.KeyNavigation.tab: secondaryButton
    primaryButton.KeyNavigation.backtab: secondaryButton

    secondaryButton.KeyNavigation.tab: primaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.visible: false
}
