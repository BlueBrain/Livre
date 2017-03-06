import QtQuick 2.0

Item {
    id: root

    property alias histogram: histogram.histogram

    Checkerboard {
        anchors.fill: parent
    }

    Histogram {
        id: histogram
        anchors.fill: parent
    }

    // data range move area
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.top: parent.top
        opacity: 0
        x: (typeof model !== "undefined" ? model.rangeX : 0) * root.width
        width: (typeof model !== "undefined" ? (model.rangeY - model.rangeX) : 0) * root.width

        // move range with drag
        onXChanged: {
            var diff = model.rangeX - x / root.width
            model.rangeX = x / root.width
            model.rangeY -= diff
        }

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.threshold: 0
            drag.minimumX: 0
            drag.maximumX: root.width - parent.width

            // increase/decrease range with mouse wheel
            onWheel: {
                var delta = (wheel.angleDelta.y / 120) * 0.01
                model.rangeX = Math.max(model.rangeX + delta, 0)
                model.rangeY = Math.min(model.rangeY - delta, 1)
            }
        }
    }

    // data range resizers
    RangeIndicator {
        objectName: "leftrange"
        anchors.fill: parent
        rangeX: typeof model !== "undefined" ? model.rangeX : 0
        rangeY: typeof model !== "undefined" ? model.rangeY : 1
        isLeft: true
        rootwidth: root.width
        rootheight: root.height
    }
    RangeIndicator {
        objectName: "rightrange"
        anchors.fill: parent
        rangeX: typeof model !== "undefined" ? model.rangeX : 0
        rangeY: typeof model !== "undefined" ? model.rangeY : 1
        isLeft: false
        rootwidth: root.width
        rootheight: root.height
    }
}
