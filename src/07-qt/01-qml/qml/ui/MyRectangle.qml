import QtQuick

Rectangle {
    id: borderRect
    property int _topMargin: 10
    property int _bottomMargin: 10

    color: "#000000"
    Rectangle {
        id: innerRect
        color: "#990099"
        anchors.fill: parent
        anchors.topMargin: borderRect._topMargin
        anchors.bottomMargin: borderRect._bottomMargin
        anchors.leftMargin: 0
        anchors.rightMargin: 0
    }
}