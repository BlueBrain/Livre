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

    // data range
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
