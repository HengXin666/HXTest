import QtQuick
import QtQuick.Controls

pragma ComponentBehavior: Bound

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("14: Hello World")

    // 明确创建焦点范围
    FocusScope {
        width: 100; height: 50;
        focus: true
        Button {
            id: btn1
            anchors.fill: parent
            focus: true
            focusPolicy: Qt.NoFocus
            background: Rectangle {
                anchors.fill: parent
                border.color: btn1.activeFocus ? "#990099" : "black"
            }
            onClicked: {
                forceActiveFocus();
            }
        }
    }

    FocusScope {
        width: 100; height: 50;
        y: 200
        focus: true
        Button {
            id: btn2
            anchors.fill: parent
            focus: true
            focusPolicy: Qt.NoFocus // 设置如何获取焦点: 默认是 Qt.NoFocus (无法获取)
                                    // 常用有: Qt.StrongFocus (鼠标和Tab可获取) 等
            background: Rectangle {
                anchors.fill: parent
                border.color: btn2.activeFocus ? "#990099" : "black"
            }
            onClicked: {
                forceActiveFocus();
            }
        }
    }
}