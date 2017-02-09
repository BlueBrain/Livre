import QtQuick 2.0
import "qrc:/qml/style.js" as Style

Canvas {
    property var histogram: []
    onHistogramChanged: requestPaint()

    id: canvas
    antialiasing: true
    onPaint: {
        var ctx = canvas.getContext('2d')
        ctx.clearRect(0, 0, canvas.width, canvas.height)

        if (parent.histogram.length == 0)
            return

        ctx.lineWidth = Style.histogramLineWidth
        ctx.strokeStyle = Style.histogramLineColor

        ctx.beginPath()
        ctx.moveTo(0, parent.histogram[0].y * parent.height)
        for (var i = 0; i < parent.histogram.length; ++i) {
            var point = parent.histogram[i]
            ctx.lineTo(point.x * parent.width, point.y * parent.height)
            ctx.lineTo(point.x * parent.width + (parent.width / parent.histogram.length),
                       point.y * parent.height)
        }
        ctx.stroke()
    }
}
