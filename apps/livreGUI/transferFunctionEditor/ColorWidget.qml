import QtQuick 2.0
import QtGraphicalEffects 1.0

Item {
    id: root

    property alias controlpoints: controlpoints.controlpoints
    property color color: "white"

    LinearGradient {
        anchors.fill: parent
        start: Qt.point(0, 0)
        end: Qt.point(0, parent.height)
        gradient: Gradient {
            GradientStop {
                position: 0
                color: root.color
            }
            GradientStop {
                position: 1
                color: "black"
            }
        }
    }

    ControlPoints {
        id: controlpoints
        anchors.fill: parent
    }
}
