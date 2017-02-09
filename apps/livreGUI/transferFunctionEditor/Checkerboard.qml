import QtQuick 2.0
import "qrc:/qml/style.js" as Style

Grid {
    id: background
    anchors.fill: parent
    rows: Math.ceil(height / Style.checkerboardCellSize)
    columns: Math.ceil(width / Style.checkerboardCellSize)
    clip: true
    Repeater {
        model: background.columns * background.rows
        Rectangle {
            property int odd_row: Math.floor(index / background.columns) % 2
            property int odd_column: (index % background.columns) % 2
            width: Style.checkerboardCellSize
            height: Style.checkerboardCellSize
            color: (odd_row == 1 ^ odd_column == 1) ? Style.checkerboardDarkGray : Style.checkerboardLightGray
        }
    }
}
