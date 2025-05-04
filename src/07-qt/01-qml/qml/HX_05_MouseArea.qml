import QtQuick

Window {
    id: container
    width: 640
    height: 480
    visible: true
    title: qsTr("05: Hello World")

    Rectangle {
        color: "yellow"
        width: 100; height: 100
        x: 100; y: 500

        MouseArea {
            anchors.fill: parent
            onClicked: console.log("clicked yellow")

            onDoubleClicked: {
                console.log("双击");
            }
        }

        Rectangle {
            color: "blue"
            width: 50; height: 50

            MouseArea {
                anchors.fill: parent
                propagateComposedEvents: true
                onClicked: (mouse) => {
                    console.log("clicked blue")
                    mouse.accepted = false
                }
            }
        }
    }

    MouseArea {
        width: 50
        height: 50
        x: 500
        y: 300
        Rectangle {
            anchors.fill: parent
            color: "#990099"
        }

        onPressAndHold: {
            console.log("长按鼠标 (800ms)");
        }
    }

    MouseArea {
        id: mouseArea_3
        anchors.left: mouseArea_2.right
        anchors.leftMargin: 20
        width: 100
        height: 100
        Rectangle {
            anchors.fill: parent
            color: "#990099"
        }
        hoverEnabled: true // 该属性用于确定是否处理悬停事件 (默认是 false)
   
        onMouseXChanged: {
            console.log("x:", mouseX);
        }
        onMouseYChanged: {
            console.log("y:", mouseY);
        }
    }
        

    Rectangle {
        width: 480
        height: 320

        y: 100
        z: 2

        Rectangle {
            x: 30; y: 30
            width: 300; height: 240
            color: "lightsteelblue"

            MouseArea {
                anchors.fill: parent
                drag.target: parent;
                drag.axis: Drag.XAxis
                drag.minimumX: 30
                drag.maximumX: 150
                drag.filterChildren: true // 子控件是否继承父控件的属性

                Rectangle {
                    color: "yellow"
                    x: 50; y : 50
                    width: 100; height: 100
                    MouseArea {
                        anchors.fill: parent
                        onClicked: console.log("Clicked")
                    }
                }
            }
        }
    }

    Rectangle {
        z: 2
        id: rect
        y: mouseArea_1.height + 20
        width: 50; height: 50
        color: "red"
        opacity: (container.width - rect.x) / container.width

        MouseArea {
            anchors.fill: parent
            drag.target: rect
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: container.width - rect.width
        }
    }

    MouseArea {
        id: mouseArea_2
        anchors.left: mouseArea_1.right
        anchors.leftMargin: 20
        width: 200
        height: 200
        Rectangle {
            anchors.fill: parent
            color: "#990099"
        }

        cursorShape: Qt.CrossCursor

        hoverEnabled: true // 该属性用于确定是否处理悬停事件 (默认是 false)
        onHoveredChanged: {
            console.log("hoverEnabled switch");
        }
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            console.log("onClicked");
        }
        onContainsMouseChanged: {
            console.log("onContainsMouseChanged", containsMouse);
        }
        onContainsPressChanged: {
            console.log("onContainsPressChanged", containsPress);
        }
    }

    MouseArea {
        id: mouseArea_1
        width: 200
        height: 200
        Rectangle {
            anchors.fill: parent
            color: "#990099"
        }

        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onClicked: {
            console.log("onClicked -> ", pressedButtons);
        }
        onPressed: {
            console.log("onPressed -> ", pressedButtons);
            if (pressedButtons & Qt.LeftButton) {
                console.log("onPressed 左键");
            } else if (pressedButtons & Qt.RightButton) {
                console.log("onPressed 右键");
            }
        }
        onReleased: {
            console.log("onReleased -> ", pressedButtons);
        }
    }
}