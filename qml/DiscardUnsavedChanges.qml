// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Dialog component shown warn the user of unsaved changes before closing the
// project
BaseDialog {
    id: control

    enum Mode { Close, OpenProject, NewProject }
    // Close: Saving changes before closing the main window
    // OpenProject: Saving changes before opening a project
    // NewProject: Saving changes before creating a new project

    property int mode: DiscardUnsavedChanges.Close

    // Set when the user chose to discard the changes before closing the main
    // window, so the window closes without showing this dialog again
    property bool discardChanges: false

    function save() {
        // Save project and hide the dialog. The next step (closing the window,
        // opening a project or creating a new project) is only taken when the
        // project has been saved successfully: when saving is cancelled or
        // fails, the unsaved changes are kept and the user is asked again.
        control.hide()
        const mainWindow = control.transientParent
        let afterSave = null
        switch( control.mode ) {
            case DiscardUnsavedChanges.Close:
                afterSave = () => { mainWindow.closeWindow() }
                break
            case DiscardUnsavedChanges.OpenProject:
                afterSave = () => { mainWindow.openProject( false ) }
                break
            case DiscardUnsavedChanges.NewProject:
                afterSave = () => { mainWindow.newProject( false ) }
                break
        }
        if ( Properties.filePathNotEmpty ) {
            mainWindow.saveProject( afterSave )
        }
        else {
            mainWindow.saveProjectAs( afterSave )
        }
    }

    function dontSave() {
        // Don't save and hide the dialog
        control.hide()
        switch( control.mode) {
            case DiscardUnsavedChanges.Close:
                control.discardChanges = true
                control.transientParent.close()
                break
            case DiscardUnsavedChanges.OpenProject:
                control.transientParent.openProject( false )
                break
            case DiscardUnsavedChanges.NewProject:
                control.transientParent.newProject ( false )
                break
        }
    }

    function cancel() {
        control.hide()
    }

    title: "Save project" +
           (
               Properties.filePathNotEmpty ?
                   (" '" + Properties.projectFileName + "'") :
                   ""
           ) +
           "?"
    headerText:
        "Save " +
        ( Properties.filePathNotEmpty ?
            ( "the changes to project '" + Properties.projectFileName + "'" ) :
            "the current project" )
        + "?"
    subText: "Your changes will be lost if you don't save them"

    primaryButton.text: "Save project"
    primaryButton.onClicked: { control.save() }
    primaryButton.KeyNavigation.tab: secondaryButton
    primaryButton.KeyNavigation.backtab: tertiaryButton

    secondaryButton.text: "Discard changes"
    secondaryButton.onClicked: { control.dontSave() }
    secondaryButton.KeyNavigation.tab: tertiaryButton
    secondaryButton.KeyNavigation.backtab: primaryButton

    tertiaryButton.text: Properties.buttonTextCancel
    tertiaryButton.onClicked: { control.cancel() }
    tertiaryButton.KeyNavigation.tab: primaryButton
    tertiaryButton.KeyNavigation.backtab: secondaryButton
}
