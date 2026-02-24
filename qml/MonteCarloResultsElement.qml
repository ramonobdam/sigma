import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

// Component used for the Monte Carlo results table right column (i.e. values)
Item {
    id: control

    property string text
    property Item table
    property alias wrapMode: text.wrapMode
    property alias lineCount: text.lineCount
    property alias maximumLineCount: text.maximumLineCount

    signal extraRequested()

    anchors {
        left: parent.left
        right: parent.right
        leftMargin: -properties.spacingM / 2
    }
    height: text.contentHeight + line.height

    ContextMenu.onRequested: position => {
        let menu = contextMenu.createObject( control, { value: control.text } )
        menu.popup( position )
    }

    Component {
        id: contextMenu

        MonteCarloContextMenu {
            table: control.table
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaText {
        id: text

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: line.top
        }

        text: control.text
        padding: properties.spacingS
        elide: maximumLineCount === 1 ? Text.ElideRight : Text.ElideNone
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignLeft
        maximumLineCount: 1
        wrapMode: Text.WordWrap
        lineHeightMode: Text.FixedHeight
        lineHeight: properties.rowHeight
    }

    Rectangle {
        id: line

        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        height: properties.borderWidth
        color: properties.colorStrokeWeak
    }
}

