// Sigma – Measurement Uncertainty Toolkit
// Copyright (c) 2025–2026 Ramon Obdam
// Licensed under the MIT License. See LICENSE file for details.

import QtQuick
import Sigma

// Button component with a lower prominence in the visual hierarchy
PrimaryButton {
    id: button

    fillColor:  button.enabled ? (
                                     button.down ? Properties.colorTextDown :
                                     (
                                          button.hovered ?
                                            Properties.colorTextHover :
                                            button.backgroundColor
                                     )
                                 ) :
                                 button.backgroundColor
    borderColor: button.textColor
    textColor: button.enabled ? (
                                    button.down ? Properties.colorDown :
                                                  (
                                                      button.hovered ?
                                                          Properties.colorHover :
                                                          Properties.colorBrand
                                                  )
                                ) :
                                Properties.colorStrokeWeak
}
