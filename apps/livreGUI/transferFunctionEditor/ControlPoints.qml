import QtQuick 2.0
import "qrc:/qml/style.js" as Style

Item {
    property var controlpoints: []

    onControlpointsChanged: canvas.requestPaint()
    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true
        onPaint: {
            var ctx = canvas.getContext('2d')
            ctx.clearRect(0, 0, canvas.width, canvas.height)

            if (parent.controlpoints.length == 0)
                return

            // draw line
            ctx.beginPath()
            ctx.lineWidth =  Style.controlpointsLineWidth
            ctx.strokeStyle = Style.controlpointsLineColor
            ctx.moveTo(0, (1 - parent.controlpoints[0].y) * parent.height)
            for (var i = 0; i < parent.controlpoints.length; ++i) {
                var point = parent.controlpoints[i]
                ctx.lineTo(point.x * parent.width,
                           (1 - point.y) * parent.height)
            }
            ctx.stroke()
            ctx.closePath()

            // draw circles
            ctx.beginPath()
            ctx.lineWidth = Style.controlpointsCircleLineWith
            ctx.fillStyle = Style.controlpointsCircleFillColor
            ctx.strokeStyle = Style.controlpointsCircleLineColor
            var circleRadius = Style.controlpointsCircleRadius
            for (var i = 0; i < parent.controlpoints.length; ++i) {
                var point = parent.controlpoints[i]
                ctx.ellipse(point.x * parent.width - circleRadius,
                            (1 - point.y) * parent.height - circleRadius,
                            circleRadius * 2, circleRadius * 2)
            }
            ctx.stroke()
            ctx.fill()
        }
    }
}
