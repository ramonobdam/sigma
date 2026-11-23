// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import Sigma
import QtQuick
import QtQuick.Dialogs

// FileDialog to select the CSV file for the Monte Carlo samples. When accepted,
// the Monte Carlo simulation is started and the samples are exported to the
// selected file once the simulation has converged. A dialog shows the result
// of the export when the simulation has finished.
FileDialog {
    id: control

    fileMode: FileDialog.SaveFile
    title: "Run Monte Carlo simulation and export samples"
    acceptLabel: "Run and export"
    rejectLabel: "Cancel"
    defaultSuffix: "csv"
    nameFilters: [ defaultSuffix + " (*." + defaultSuffix + ")" ]
    onAccepted: { calculation.runMonteCarlo( selectedFile ) }

    Connections {
        target: calculation

        function onSampleExportFinished( exported, message ) {
            resultDialog.exported = exported
            resultDialog.subText = message
            resultDialog.show()
        }
    }

    BaseDialog {
        id: resultDialog

        property bool exported: false

        transientParent: control.parentWindow

        errorIcon: !exported
        iconText: exported ? Properties.circleCheckIcon :
                             Properties.circleXmarkIcon
        iconColor: exported ? Properties.colorSuccess500 :
                              Properties.colorError500
        title: exported ? "Samples exported" : "Samples could not be exported"
        headerText: exported ?
                        "Monte Carlo samples exported" :
                        "Monte Carlo samples could not be exported"

        primaryButton.text: Properties.buttonTextOK
        primaryButton.onClicked: hide()

        secondaryButton.visible: false
        tertiaryButton.visible: false
    }
}
