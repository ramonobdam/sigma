// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Controls.Material.impl
import Sigma

// SplitView component with custom styling
SplitView {
    id: control

    property int handleWidth: Properties.spacingM
    property int indicatorThickness: Properties.splitHandleIndicatorThickness
    property int animationDuration: Properties.animationDuration

    handle: Item {
        id: handle

        property bool pressed: SplitHandle.pressed
        property bool hovered: SplitHandle.hovered

        implicitWidth: orientation === Qt.Horizontal ? control.handleWidth :
                                                       control.width
        implicitHeight: orientation === Qt.Horizontal ? control.height :
                                                        control.handleWidth

        Rectangle {
            property int margin:
                ( control.handleWidth - control.indicatorThickness ) / 2
            property int spacing: Properties.radiusM

            anchors {
                fill: parent
                leftMargin: orientation === Qt.Horizontal ? margin : spacing
                rightMargin: orientation === Qt.Horizontal ? margin : spacing
                topMargin: orientation === Qt.Horizontal ? spacing : margin
                bottomMargin: orientation === Qt.Horizontal ? spacing : margin
            }
            color: handle.hovered ? Properties.colorStrokeWeak :
                                    Properties.colorBase
        }

        Rectangle {
            id: indicator

            property int length:
                handle.pressed ? Properties.splitHandleIndicatorInitialLength *
                                 Properties.splitHandleIndicatorScaleFactor :
                                 Properties.splitHandleIndicatorInitialLength

            color: handle.pressed ?
                       Properties.colorDown :
                       (
                           handle.hovered ?
                               Properties.colorHover :
                               Properties.colorBrand
                       )
            width: orientation === Qt.Horizontal ? control.indicatorThickness :
                                                   length
            height: orientation === Qt.Horizontal ? length :
                                                    control.indicatorThickness
            radius: control.indicatorThickness / 2
            x: ( parent.width - width ) / 2
            y: ( parent.height - height ) / 2

            Behavior on length {
                NumberAnimation {
                    duration: control.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: control.animationDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
