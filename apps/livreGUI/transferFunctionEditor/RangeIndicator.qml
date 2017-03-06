import QtQuick 2.0
import "qrc:/qml/style.js" as Style

Item {
    id: root
    property bool isLeft: true
    property real rangeX: 0
    property real rangeY: 1
    property int rootwidth: 1
    property int rootheight: 1

    signal viewposChanged(real value)

    Rectangle {
        color: Style.rangeIndicatorRectangleColor
        opacity: Style.rangeIndicatorRectangleOpacity
        anchors.left: parent.isLeft ? parent.left : undefined
        anchors.right: !parent.isLeft ? parent.right : undefined
        height: rootheight
        width: parent.isLeft ? circle.x + circle.width / 2 : rootwidth - circle.x - circle.width / 2
    }

    Rectangle {
        id: circle
        color: Style.rangeIndicatorCircleColor
        opacity: Style.rangeIndicatorCircleOpacity
        border.width: Style.rangeIndicatorCircleLineWidth
        width: Style.rangeIndicatorCircleRadius
        height: Style.rangeIndicatorCircleRadius
        radius: Style.rangeIndicatorCircleRadius

        x: (parent.isLeft ? parent.rangeX : parent.rangeY) * rootwidth - width / 2
        y: rootheight / 2 - height / 2

        onXChanged: parent.viewposChanged((x + width / 2) / rootwidth)

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: (root.isLeft ? 0 : root.rangeX * rootwidth) - parent.width / 2
            drag.maximumX: (root.isLeft ? root.rangeY * rootwidth : rootwidth) - parent.width / 2
            drag.threshold: 0
        }
    }
}
