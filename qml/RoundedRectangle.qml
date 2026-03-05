import QtQuick

// The rounded rectangle works mostly like a regular rectangle, but provides the
// option to have rounded corners on only one side of the rectangle.

Canvas {
    id: canvas

    property color color: "red"
    property int radius: 0

    // On what side should the corners be shown 5 can be used if no radius is needed.
    // 1 is down, 2 is left, 3 is up and 4 is right.
    property int cornerSide: RoundedRectangle.Direction.None

    // Simple object to ensure that border.width and border.color work
    property alias border: border
    // Extra aliases are created to have signals when border width/color change
    property alias borderColor: border.color
    property alias borderWidth: border.width

    BorderGroup {
        id: border
    }

    enum Direction {
        None = 0,
        Down = 1,
        Left = 2,
        Up = 3,
        Right = 4,
        All = 5
    }

    antialiasing: true

    onPaint: {
        var ctx = getContext( "2d" )

        var w = width
        var h = height
        var r = Math.min( radius, Math.min( w, h ) / 2 )

        ctx.reset()

        ctx.clearRect( 0, 0, w, h )

        // Half-pixel correction for crisp borders
        var offset = border.width > 0 ? border.width / 2 : 0

        ctx.beginPath()

        // Path construction
        ctx.moveTo(
            offset +
            (
                (
                    cornerSide === RoundedRectangle.Direction.Left ||
                    cornerSide === RoundedRectangle.Direction.Up ||
                    cornerSide === RoundedRectangle.Direction.All
                ) ? r : 0
            ),
            offset
        )

        // top edge
        ctx.lineTo(
            w - offset -
            (
                (
                    cornerSide === RoundedRectangle.Direction.Right ||
                    cornerSide === RoundedRectangle.Direction.Up ||
                    cornerSide === RoundedRectangle.Direction.All
                ) ? r : 0
            ),
            offset
        )

        if (
                cornerSide === RoundedRectangle.Direction.Right ||
                cornerSide === RoundedRectangle.Direction.Up ||
                cornerSide === RoundedRectangle.Direction.All
        ) {
            ctx.arc( w - offset - r, offset + r, r, -Math.PI/2, 0 )
        }

        // right edge
        ctx.lineTo(
            w - offset,
            h - offset -
            (
                (
                    cornerSide === RoundedRectangle.Direction.Right ||
                    cornerSide === RoundedRectangle.Direction.Down ||
                    cornerSide === RoundedRectangle.Direction.All
                ) ? r : 0
            )
        )

        if (
                cornerSide === RoundedRectangle.Direction.Right ||
                cornerSide === RoundedRectangle.Direction.Down ||
                cornerSide === RoundedRectangle.Direction.All
        ) {
            ctx.arc( w - offset - r, h - offset - r, r, 0, Math.PI/2 )
        }

        // bottom edge
        ctx.lineTo(
            offset +
            (
                (
                    cornerSide === RoundedRectangle.Direction.Left ||
                    cornerSide === RoundedRectangle.Direction.Down ||
                    cornerSide === RoundedRectangle.Direction.All
                ) ? r : 0
            ),
            h - offset
        )

        if (
                (
                    cornerSide === RoundedRectangle.Direction.Left ||
                    cornerSide === RoundedRectangle.Direction.Down ||
                    cornerSide === RoundedRectangle.Direction.All
                )
        ) {
            ctx.arc( offset + r, h - offset - r, r, Math.PI/2, Math.PI )
        }

        // left edge
        ctx.lineTo(
            offset,
            offset +
            (
                (
                    cornerSide === RoundedRectangle.Direction.Left ||
                    cornerSide === RoundedRectangle.Direction.Up ||
                    cornerSide === RoundedRectangle.Direction.All
                ) ? r : 0
            )
        )

        if (
                (
                    cornerSide === RoundedRectangle.Direction.Left ||
                    cornerSide === RoundedRectangle.Direction.Up ||
                    cornerSide === RoundedRectangle.Direction.All
                )
        ) {
            ctx.arc( offset + r, offset + r, r, Math.PI, 3 * Math.PI/2 )
        }

        ctx.closePath()

        // fill
        ctx.fillStyle = color
        ctx.fill()

        // border
        if ( border.width > 0 ) {
            ctx.lineWidth = border.width
            ctx.strokeStyle = border.color
            ctx.stroke()
        }
    }

    onWidthChanged: { requestPaint() }
    onHeightChanged: { requestPaint() }
    onColorChanged: { requestPaint() }
    onRadiusChanged: { requestPaint() }
    onCornerSideChanged: { requestPaint() }
    onBorderColorChanged: { requestPaint() }
    onBorderWidthChanged: { requestPaint() }
}
