// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl

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
    property alias properties: properties

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
    flags: properties.dialogFlags
    color: properties.colorTransparent
    onVisibleChanged: {
        if ( visible ) {
            Qt.callLater( centerOnParent )
            primaryButton.forceActiveFocus()
        }
    }

    SigmaProperties {
        id: properties
    }

    SigmaFonts {
        id: fonts
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
            margins: properties.spacingM
        }
        width: 235
        height: contentColumn.implicitHeight + padding * 2
        padding: properties.spacingM

        background: Rectangle {
            color: properties.colorElevated
            radius: properties.radiusM
            border.width: properties.borderWidth
            border.color: properties.colorStrokeWeak
            layer.enabled: true
            layer.effect: RoundedElevationEffect {
                elevation: properties.elevationElevated
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
                height: properties.spacingS
            }

            Text {
                id: icon
                width: parent.width
                height: undefined
                text: errorIcon ? properties.circleXmarkIcon :
                                  properties.triangleExclamationIcon
                font.family: fonts.fontAwesome.font.family
                font.pixelSize: properties.fontSizeDialogIcon
                color: errorIcon ? properties.colorError500 :
                                   properties.colorWarning500
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Spacer {
                height: properties.spacingM
            }

            Text {
                id: header
                width: parent.width
                height: undefined
                color: properties.colorTextStrong
                font.family: fonts.interSemiBold.font.family
                font.bold: true
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 10
            }

            Spacer {
                height: properties.spacingS
            }

            Text {
                id: text
                width: parent.width
                height: undefined
                color: properties.colorTextWeak
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                maximumLineCount: 10
            }

            Spacer {
                height: primaryButton.visible ? properties.spacingM : 0
            }

            PrimaryButton {
                id: primaryButton
                width: parent.width
                KeyNavigation.up: KeyNavigation.backtab
                KeyNavigation.down: KeyNavigation.tab
            }

            Spacer {
                height: secondaryButton.visible ? properties.spacingButtons : 0
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
                height: tertiaryButton.visible ? properties.spacingButtons : 0
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
