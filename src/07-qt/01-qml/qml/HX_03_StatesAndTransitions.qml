import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("03: Hello World")

    // 依次的动画
    Rectangle {
        id: rect4
        width: 150
        height: 100
        x: 600
        y: 300
        color: "#8b8b8b"

        Column {
            anchors.centerIn: parent
            Text {
                id: text_01
                text: qsTr("这个是文本1")
                opacity: 0
            }
            Text {
                id: text_02
                text: qsTr("这个是文本2")
                opacity: 0
            }
            Text {
                id: text_03
                text: qsTr("这个是文本3")
                opacity: 0
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: playBanner.start()
        }

        SequentialAnimation {
            id: playBanner
            running: false // 是否自动触发
            NumberAnimation { target: text_01; property: "opacity"; from: 0; to: 1; duration: 1000 }
            NumberAnimation { target: text_02; property: "opacity"; from: 0; to: 1; duration: 1000 }
            NumberAnimation { target: text_03; property: "opacity"; from: 0; to: 1; duration: 1000 }
        }
    }

    // 值改变而触发的动画
    Rectangle {
        id: rect3
        width: 75
        height: 75
        x: 600
        y: 200
        color: "#990099"
        MouseArea {
            anchors.fill: parent
            onPressed: {
                rect3.x -= 30;
                rect3.y += 20;
            }
        }

        Behavior on y {
            NumberAnimation {
                id: bouncebehavior
                easing {
                    type: Easing.OutElastic
                    amplitude: 1.0
                    period: 0.5
                }
            }
        }
    }

    // 动画过渡
    Rectangle {
        id: rect2
        width: 75
        height: 75
        x: 600
        state: "state_01"

        states: [
            State {
                name: "state_01"
                PropertyChanges {
                    rect2.color: "#990099"
                }
            },
            State {
                name: "state_02"
                PropertyChanges {
                    rect2.color: "#2b2b2b"
                }
            }
        ]

        transitions: [
            Transition {
                ColorAnimation {
                    target: rect2
                    duration: 1000
                }
            }
        ]

        MouseArea {
            anchors.fill: parent
            onReleased: rect2.state = "state_01"
            onPressed: rect2.state = "state_02"
        }
    }

    // 动画队列
    Rectangle {
        width: 75
        height: 75
        x: 500
        SequentialAnimation on color {
            ColorAnimation { to: "yellow"; duration: 1000 }
            ColorAnimation { to: "#990099"; duration: 1000 }
            ColorAnimation { to: "blue"; duration: 1000 }
        }
        SequentialAnimation on y {
            NumberAnimation { from: 100; to: 300; duration: 2000 }
            NumberAnimation { to: 100; duration: 2000 }
        }
    }

    // 颜色动画
    Rectangle {
        width: 75
        height: 75
        x: 300
        y: 300
        ColorAnimation on color {
            from: "#990099"
            to: "#2b2b2b"
            duration: 1000
        }
    }

    // 立刻生效的动画
    Rectangle {
        width: 75
        height: 75
        color: "#990099"

        PropertyAnimation on x {
            from: 75
            to: 175
            duration: 1000
        }
        PropertyAnimation on y {
            from: 75
            to: 175
            duration: 1000
        }
        NumberAnimation on opacity {
            from: 0.1
            to: 1.0
            duration: 2000
        }
    }

    // 动画示例
    Rectangle {
        id: flashingblob
        width: 75
        height: 75
        color: "blue"
        opacity: 1.0

        MouseArea {
            anchors.fill: parent
            onClicked: {
                animateColor.start();
                animateOpacity.start();
                animateWidth.start();
            }
        }

        // 定义一个属性动画
        PropertyAnimation {
            id: animateColor        // 动画名称(id)
            target: flashingblob    // 作用对象(id)
            properties: "color"     // 需要改变的属性名称
            to: "green"             // 从当前值改为的目标值
            duration: 1000          // 持续时间: ms
        }
        // 定义一个数值动画
        NumberAnimation {
            id: animateOpacity      // 动画名称(id)
            target: flashingblob    // 作用对象(id)
            properties: "opacity"   // 需要改变的属性名称
            from: 0.1               // 初始值
            to: 1.0                 // 目标值
            duration: 2000          // 持续时间: ms
        }
        // 可定义多个, 通过 id 调用
        NumberAnimation {
            id: animateWidth
            target: flashingblob
            properties: "width"
            from: 100
            to: 175
            duration: 1000
        }
    }

    // 状态示例
    Rectangle {
        id: root
        anchors.centerIn: parent
        width: 50
        height: 50

        states: [
            State {
                name: "red_color"
                PropertyChanges { root.color: "red"; root.width: 200; }
            },
            State {
                name: "blue_color"
                PropertyChanges { root.color: "blue"; root.height: 200; }
            }
        ]
        state: "red_color"

        MouseArea {
            anchors.fill: parent
            onPressed: {
                root.state = "blue_color"
            }
            onReleased: {
                root.state = "red_color"
            }
        }
    }
}