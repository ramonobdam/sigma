// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import Sigma
import QtQuick

// Dialog to inform that saving or opening the project failed. When saving
// failed (e.g. a locked project file), the project can be saved to another file
// instead.
BaseDialog {
    id: control

    property bool save: true
    property url fileURL
    readonly property string fileName: calculation.urlToFileName( fileURL )
    // Optional function of the failed save (e.g. closing the window or opening
    // a project), taken after saving to another file succeeded
    property var afterSave: null

    function saveAs() {
        const pendingAction = control.afterSave
        control.afterSave = null
        control.hide()
        control.transientParent.saveProjectAs( pendingAction )
    }

    function cancel() {
        control.afterSave = null
        control.hide()
    }

    title:  "Project could not be " + ( control.save ? "saved" : "opened" )

    errorIcon: true
    headerText: "Project " +
                ( control.fileName.length > 0 ?
                      "'" + control.fileName + "' " : "" ) +
                "could not be " + ( control.save ? "saved" : "opened" )
    subText: control.save ?
        "Please make sure the file is not locked and the disk is not full, " +
        "or save the project to another file" :
        "Please make sure the selected file is readable"

    primaryButton.text: control.save ? "Save project as..." :
                                       Properties.buttonTextOK
    primaryButton.onClicked: {
        if ( control.save ) {
            control.saveAs()
        }
        else {
            control.cancel()
        }
    }
    primaryButton.KeyNavigation.tab: control.save ? secondaryButton : null
    primaryButton.KeyNavigation.backtab: control.save ? secondaryButton : null

    // BaseDialog's secondary button ('Cancel') hides the dialog
    secondaryButton.visible: control.save
    secondaryButton.onClicked: { control.afterSave = null }
    secondaryButton.KeyNavigation.tab: primaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.visible: false
}
