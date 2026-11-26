// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs
import Sigma

// Dialog component to save a project
FileDialog {
    property bool save: fileMode === FileDialog.SaveFile
    property bool success: false
    // Optional function called after the project has been saved successfully
    // (e.g. closing the window or opening a project). It's dropped when saving
    // is cancelled, and passed on to the save-failed dialog when saving fails
    // (to be taken after saving to another file).
    property var afterSave: null

    title: acceptLabel
    acceptLabel: ( save ? "Save" : "Open" ) + " project"
    rejectLabel: Properties.buttonTextCancel
    defaultSuffix: Properties.fileExtension
    nameFilters: [ defaultSuffix + " (*." + defaultSuffix + ")" ]
    onAccepted: {
        if ( save ) {
            success = calculation.saveProject( selectedFile )
            const pendingAction = afterSave
            afterSave = null
            if ( success && pendingAction ) {
                // Called later, since the action may open this dialog again
                Qt.callLater( pendingAction )
            }
            else if ( !success ) {
                // The next step is kept for saving to another file
                projectFailed.save = true
                projectFailed.fileURL = selectedFile
                projectFailed.afterSave = pendingAction
                projectFailed.show()
            }
        }
        else {
            if ( !calculation.loadProject( selectedFile ) ) {
                projectFailed.save = false
                projectFailed.fileURL = selectedFile
                projectFailed.afterSave = null
                projectFailed.show()
            }
        }
    }
    onRejected: { afterSave = null }
}
