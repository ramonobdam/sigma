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
    property bool closeWindowOnSuccess: false

    title: acceptLabel
    acceptLabel: ( save ? "Save" : "Open" ) + " project"
    rejectLabel: properties.buttonTextCancel
    defaultSuffix: properties.fileExtension
    nameFilters: [ defaultSuffix + " (*." + defaultSuffix + ")" ]
    onAccepted: {
        if ( save ) {
            success = calculation.saveProject( selectedFile )
            if ( success && closeWindowOnSuccess ) {
                window.close()
            }
            else if ( !success ) {
                projectFailed.save = true
                projectFailed.fileURL = selectedFile
                projectFailed.show()
            }
        }
        else {
            if ( !calculation.loadProject( selectedFile ) ) {
                projectFailed.save = false
                projectFailed.fileURL = selectedFile
                projectFailed.show()
            }
        }
    }

    SigmaProperties {
        id: properties
    }
}
