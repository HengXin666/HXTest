import QtQuick
import "ui"

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("02: Hello World")

    MyRectangle {
        width: 50
        height: 50
        x: 400
        y: 400
    }

    // 旋转 + 放缩 + 抗锯齿
    Rectangle {
        width: 50
        height: 50
        color: "#666abc"
        anchors.centerIn: parent
        rotation: 45 // 旋转
        scale: 5     // 放缩 2 倍
        antialiasing: true // 抗锯齿开
        border.width: 10 // 边框宽度
        radius: 10 // 圆角

        gradient: Gradient {
            GradientStop { position: 0.0; color: "lights teelblue" }
            GradientStop { position: 1.0; color: "blue" }
        }
    }

    // 居中
    Rectangle {
        width: 50
        height: 50
        color: "#666abc"
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // 基于锚点的写法
    Rectangle {
        id: rect3
        x: 50
        y: 300
        width: 50
        height: 50
        color: "#3c3c3c"
    }

    Rectangle {
        id: rect4
        anchors.left: rect3.right // 左边对齐 rect3 的右边
        anchors.top: rect3.top    // 与 rect3 的上边对齐 (不然y就是0了)
        anchors.leftMargin: 20    // 外边距是 20
        width: 50
        height: 50
        color: "#3c3c3c"
    }


    // 普通基于绝对坐标的写法
    Rectangle {
        id: rect1
        x: 50
        y: 200
        width: 50
        height: 50
        color: "#3c3c3c"
    }

    Rectangle {
        id: rect2
        x: rect1.x + 75
        y: 200
        width: 50
        height: 50
        color: "#3c3c3c"
    }

    // z-index 示例:
    Rectangle {
        x: 50
        y: 50
        z: 2
        width: 50
        height: 50

        color: "#990099"
    }

    Rectangle {
        x: 75
        y: 75
        z: 1
        width: 50
        height: 50

        color: "#03f"
    }
}