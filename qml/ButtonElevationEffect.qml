import QtQuick.Controls.Material.impl

// Component that provides the shadow effect on buttons
ElevationEffect {
    _shadows: [ {
        angularValues: [
            { offset: 0, blur: 1, spread: 0 },
            { offset: 1, blur: 1, spread: 0 },
            { offset: 2, blur: 1, spread: -1 }
        ],
        strength: 0.05
    } ]
}
