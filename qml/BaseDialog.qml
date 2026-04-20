// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// Base dialog component that centers on its parent window with no window frame
// and custom styling
Window {
    id: control

    property bool errorIcon: false
    property alias iconText: icon.text
    property alias iconColor: icon.color
    property alias headerText: header.text
    property alias headerMaximumLineCount: header.maximumLineCount
    property alias subText: text.text
    property alias subTextMaximumLineCount: text.maximumLineCount
    property alias primaryButton: primaryButton
    property alias secondaryButton: secondaryButton
    property alias tertiaryButton: tertiaryButton

    function centerOnParent() {
        // Center the window on its transient parent
        if ( !transientParent ) {
            return
        }

        // Synchronize the active screen to transient parent
        if ( control.screen !== transientParent.screen ) {
            control.screen = transientParent.screen
        }

        // Determine position with respect to active screen
        var s = control.screen

        control.x = s.virtualX
                   + (transientParent.x - s.virtualX)
                   + (transientParent.width - control.width) / 2

        control.y = s.virtualY
                   + (transientParent.y - s.virtualY)
                   + (transientParent.height - control.height) / 2
    }

    width: pane.implicitWidth + pane.anchors.margins * 2
    height: pane.implicitHeight + pane.anchors.margins * 2
    maximumWidth: width
    maximumHeight: height
    minimumWidth: width
    minimumHeight: height
    modality: Qt.ApplicationModal
    flags: Properties.dialogFlags
    color: Properties.colorTransparent
    onVisibleChanged: {
        if ( visible ) {
            Qt.callLater( centerOnParent )
            primaryButton.forceActiveFocus()
        }
    }

    Connections {
        // Reposition when the transient parent changes position, size or screen
        target: transientParent

        function onXChanged() { centerOnParent() }
        function onYChanged() { centerOnParent() }
        function onWidthChanged() { centerOnParent() }
        function onHeightChanged() { centerOnParent() }
        function onScreenChanged() { centerOnParent() }
    }

    Pane {
        id: pane

        anchors {
            top: parent.top
            left: parent.left
            margins: Properties.spacingM
        }
        width: 235
        height: contentColumn.implicitHeight + padding * 2
        padding: Properties.spacingM

        background: Rectangle {
            color: Properties.colorElevated
            radius: Properties.radiusM
            border.width: Properties.borderWidth
            border.color: Properties.colorStrokeWeak
            layer.enabled: true
            layer.effect: RoundedElevationEffect {
                elevation: Properties.elevationElevated
                roundedScale: Material.ExtraSmallScale
            }
        }
        focus: true
        Keys.onEscapePressed: { control.hide() }

        Column {
            id: contentColumn

            width: parent.width
            spacing: 0

            Spacer {
                height: Properties.spacingS
            }

            Text {
                id: icon
                width: parent.width
                height: undefined
                text: errorIcon ? Properties.circleXmarkIcon :
                                  Properties.triangleExclamationIcon
                font.family: Fonts.fontAwesome.font.family
                font.pixelSize: Properties.fontSizeDialogIcon
                color: errorIcon ? Properties.colorError500 :
                                   Properties.colorWarning500
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Spacer {
                height: Properties.spacingM
            }

            Text {
                id: header
                width: parent.width
                height: undefined
                color: Properties.colorTextStrong
                font.family: Fonts.interSemiBold.font.family
                font.bold: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 10
            }

            Spacer {
                height: Properties.spacingS
            }

            Text {
                id: text
                width: parent.width
                height: undefined
                color: Properties.colorTextWeak
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 10
            }

            Spacer {
                height: primaryButton.visible ? Properties.spacingM : 0
            }

            PrimaryButton {
                id: primaryButton
                width: parent.width
                KeyNavigation.up: KeyNavigation.backtab
                KeyNavigation.down: KeyNavigation.tab
            }

            Spacer {
                height: secondaryButton.visible ? Properties.spacingButtons : 0
            }

            SecondaryButton {
                id: secondaryButton
                text: "Cancel"
                width: parent.width
                onClicked: { control.hide() }
                KeyNavigation.up: KeyNavigation.backtab
                KeyNavigation.down: KeyNavigation.tab
            }

            Spacer {
                height: tertiaryButton.visible ? Properties.spacingButtons : 0
            }

            SecondaryButton {
                id: tertiaryButton
                width: parent.width
                KeyNavigation.up: KeyNavigation.backtab
                KeyNavigation.down: KeyNavigation.tab
            }
        }
    }
}
