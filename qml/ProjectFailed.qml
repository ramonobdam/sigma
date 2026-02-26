// Dialog to inform that saving the project failed
BaseDialog {
    id: control

    property bool save: true
    property url fileURL
    property string filePath: ""

    onFileURLChanged: {
        // Convert fileURL to filePath
        let path = decodeURIComponent( control.fileURL.toString() )
        if ( path.startsWith( "file:///" ) ) {
            control.filePath = path.substring( 8 )
        }
        else if ( path.startsWith( "file://" ) ) {
            control.filePath = path.substring( 7 )
        }
    }

    title:  "Project could not be " + ( control.save ? "saved" : "opened" )

    errorIcon: true
    headerText: "Project '" + control.filePath + "' could not be " +
                ( control.save ? "saved" : "opened" )
    subText: control.save ?
        "Please make sure the file is not locked and the disk is not full" :
        "Please make sure the selected file is readable"

    primaryButton.text: properties.buttonTextOK
    primaryButton.onClicked: { control.hide() }

    secondaryButton.visible: false

    tertiaryButton.visible: false
}
