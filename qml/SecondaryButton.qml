import QtQuick

// Button component with a lower prominence in the visual hierarchy
PrimaryButton {
    id: button

    fillColor:  button.enabled ? (
                                     button.down ? properties.colorTextDown :
                                     (
                                          button.hovered ?
                                            properties.colorTextHover :
                                            button.backgroundColor
                                     )
                                 ) :
                                 button.backgroundColor
    borderColor: button.textColor
    textColor: button.enabled ? (
                                    button.down ? properties.colorDown :
                                                  (
                                                      button.hovered ?
                                                          properties.colorHover :
                                                          properties.colorBrand
                                                  )
                                ) :
                                properties.colorStrokeWeak

    SigmaProperties {
        id: properties
    }
}
