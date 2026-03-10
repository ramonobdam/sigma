// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtCore
import Sigma

// Table containing the Monte Carlo results. A SplitView is used to allow
// resizing of the columns.
SplitView {
    id: control

    signal extraRequested()

    anchors {
        top: parent.top
        left: parent.left
        right: parent.right
    }
    height: rightColumn.implicitHeight

    handle: Rectangle {
        implicitWidth: properties.spacingM
        implicitHeight: control.height
        color: properties.colorTransparent
    }

    Component.onCompleted: restoreState( settings.svMonte )
    Component.onDestruction: settings.svMonte = saveState()

    Settings {
        // Store persistent settings
        id: settings

        // SplitView state
        property var svMonte
    }

    SigmaProperties {
        id: properties
    }

    Column {
        id: leftColumn

        spacing: 0
        SplitView.minimumWidth: properties.minWidthMonteCarloTableHeader
        SplitView.preferredWidth: properties.minWidthMonteCarloTableHeader

        MonteCarloResultsHeader {
            height: mCRE1.height
            text: calculation.getMonteCarloHeader( 0 )
        }

        MonteCarloResultsHeader {
            height: mCRE2.height
            text: calculation.getMonteCarloHeader( 1 )
        }

        MonteCarloResultsHeader {
            height: mCRE3.height
            text: calculation.getMonteCarloHeader( 2 )
        }

        MonteCarloResultsHeader {
            height: mCRE4.height
            text: calculation.getMonteCarloHeader( 3 )
        }

        MonteCarloResultsHeader {
            height: mCRE5.height
            text: calculation.getMonteCarloHeader( 4 )
        }

        MonteCarloResultsHeader {
            height: mCRE6.height
            text: calculation.getMonteCarloHeader( 5 )
        }

        MonteCarloResultsHeader {
            height: mCRE7.height
            text: calculation.getMonteCarloHeader( 6 )
        }

        MonteCarloResultsHeader {
            height: mCRE8.height
            text: calculation.getMonteCarloHeader( 7 )
        }
    }

    Column {
        id: rightColumn

        spacing: 0
        SplitView.minimumWidth:  properties.minWidthMonteCarloTableHeader
        SplitView.fillWidth: true

        MonteCarloResultsElement {
            id: mCRE1

            text: calculation.monteCarloResultsList[ 0 ]
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE2

            text: calculation.monteCarloResultsList[ 1 ]
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE3

            text: calculation.monteCarloResultsList[ 2 ]
            maximumLineCount: 3
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE4

            text: calculation.monteCarloResultsList[ 3 ]
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE5

            text: calculation.monteCarloResultsList[ 4 ]
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE6

            text: calculation.monteCarloResultsList[ 5 ]
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE7

            text: calculation.monteCarloResultsList[ 6 ]
            maximumLineCount: 2
            table: control
        }

        MonteCarloResultsElement {
            id: mCRE8

            text: calculation.monteCarloResultsList[ 7 ]
            table: control
        }
    }
}
