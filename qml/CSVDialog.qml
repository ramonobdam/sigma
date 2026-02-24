import Sigma
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs

// FileDialog to save a CSV file. Shows a dialog in case the file could not be
// saved.
FileDialog {
    id: control

    property string fileName: ""

    fileMode: FileDialog.SaveFile
    title: "Save CSV file"
    acceptLabel: "Save CSV"
    rejectLabel: "Cancel"
    defaultSuffix: "csv"
    nameFilters: [ defaultSuffix + " (*." + defaultSuffix + ")" ]
    onAccepted: {
        if ( !calculation.saveCSV( selectedFile ) ) {
            var urlObject = new URL( selectedFile )
            let path = urlObject.pathname
            fileName = path.substring( path.lastIndexOf( '/' ) + 1 )
            csvFailedDialog.show()
        }
    }

    BaseDialog {
        id: csvFailedDialog
        transientParent: control.parentWindow

        errorIcon: true
        title: "CSV file could not be saved"
        headerText: "The file " +
                    (
                        control.fileName.length > 0 ?
                            "'" + control.fileName + "'" :
                            ""
                    ) +
                    " could not be saved"
        subText:
            "Please make sure the file is not locked and the disk is not full"

        primaryButton.text: "OK"
        primaryButton.onClicked: hide()

        secondaryButton.visible: false
        tertiaryButton.visible: false
    }
}
