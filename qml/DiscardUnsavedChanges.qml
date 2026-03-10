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

    // Close main window after saving the project?
    property bool closeWindow: false

    function save() {
        // Save project and hide the dialog
        control.hide()
        control.closeWindow = ( control.mode === DiscardUnsavedChanges.Close )
        if ( properties.filePathNotEmpty ) {
            control.transientParent.saveProject( control.closeWindow )
        }
        else {
            control.transientParent.saveProjectAs( control.closeWindow )
        }
        if ( control.mode === DiscardUnsavedChanges.OpenProject ) {
            control.transientParent.openProject( false )
        }
        else if ( control.mode === DiscardUnsavedChanges.NewProject) {
            control.transientParent.newProject( false )
        }
    }

    function dontSave() {
        // Don't save and hide the dialog
        control.hide()
        switch( control.mode) {
            case DiscardUnsavedChanges.Close:
                control.closeWindow = true
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
        control.closeWindow = false
        control.hide()
    }

    title: "Save project" +
           (
               properties.filePathNotEmpty ?
                   (" '" + properties.projectFileName + "'") :
                   ""
           ) +
           "?"
    headerText:
        "Save " +
        ( properties.filePathNotEmpty ?
            ( "the changes to project '" + properties.projectFileName + "'" ) :
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

    tertiaryButton.text: properties.buttonTextCancel
    tertiaryButton.onClicked: { control.cancel() }
    tertiaryButton.KeyNavigation.tab: primaryButton
    tertiaryButton.KeyNavigation.backtab: secondaryButton
}
