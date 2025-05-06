import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("10: Hello World")

    Button {
        text: "Open"
        onClicked: {
            console.log("弹出窗口");
            popup.open(); // 显示窗口的方法一: .open()
                          // 或者 手动设置其 visible 属性
        }
    }

    Popup { // 类似于 Rectangle
        id: popup
        x: 100
        y: 100
        width: 200
        height: 300
        modal: true // 模态: true (默认是非模态)
        focus: true // 焦点
        dim: true   // 控制是否调暗背景
        // closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent

        Component.onCompleted: {
            console.log("可见:", visible); // 默认 visible = false
        }

        // 窗口弹出时候的动画
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0; duration: 1000 }
        }
        // 窗口关闭时候的动画
        exit: Transition {
            NumberAnimation { property: "opacity"; from: 1.0; to: 0.0; duration: 1000 }
        }

        // 自定义模态背景色
        Overlay.modal: Rectangle {
            anchors.fill: parent
            color: "#64990099"
        }

        contentItem: Rectangle {
            anchors.fill: parent
            Text {
                text: "Msg HX: 114514"
            }
            Button {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                text: "确定"
                onClicked: console.log("确定")
            }
        }
    }

    Rectangle {
        y: 200;
        width: 100; height: 100;
        color: "#990099"
        visible: false
        Rectangle {
            width: 50; height: 50;
            color: "#2b2b2b"
            visible: true
        }
    }
}