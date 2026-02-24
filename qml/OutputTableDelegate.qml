import QtQuick
import QtQml.Models

// Specialized TableDelegate for the output parameters table
DelegateChooser {
    // The first column TableDelegate shows the valid/invalid icon
    DelegateChoice {
        column: 0
        ValidTableDelegate {
            columnWidthProvider:
                function( column ) {
                    return getOutputColumnWidth( column )
                }
            TableView.editDelegate:
                Component {
                    Item {
                        Component.onCompleted: editOutputParam.open( true )
                    }
                }
        }
    }

    // Default TableDelegate
    DelegateChoice {
        DefaultTableDelegate {
            columnWidthProvider:
                function( column ) {
                    return getOutputColumnWidth( column )
                }
            TableView.editDelegate:
                Component {
                    Item {
                        Component.onCompleted: editOutputParam.open( true )
                    }
                }
        }
    }
}
