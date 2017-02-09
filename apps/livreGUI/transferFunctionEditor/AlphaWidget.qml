import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: root

    property alias gradient: gradient.gradient
    property alias histogram: histogram.histogram
    property var colors: []
    property var positions: []
    property alias controlpoints: controlpoints.controlpoints

    // interestingly enough there is no proper way to dynamically update
    // gradients. Hence, if we have new stops, we just create a new (hard-coded)
    // one and replace the old one...
    onColorsChanged: {
        var code = 'import QtQuick 2.0; Gradient {'
        for (var i = 0; i < colors.length; i++) {
            var colorVec = colors[i]
            var position = positions[i]
            code += 'GradientStop { position: ' + position + '; color: Qt.rgba('
                    + colorVec.x + ', ' + colorVec.y + ', ' + colorVec.z + ',' + colorVec.w + ')}'
        }
        code += '}'
        var o = Qt.createQmlObject(code, root, 'qml')
        root.gradient.destroy()
        root.gradient = o
    }

    Checkerboard {
        anchors.fill: parent
    }

    LinearGradient {
        id: gradient
        anchors.fill: parent
        start: Qt.point(0, 0)
        end: Qt.point(parent.width, 0)
        gradient: Gradient {}
    }

    Histogram {
        id: histogram
        anchors.fill: parent
    }

    ControlPoints {
        id: controlpoints
        anchors.fill: parent
    }
}
