import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("08: Hello World")

    ButtonGroup {
        id: bg
        exclusive: true // 是否排他 (默认为 true)
        // buttons: column.children
    }

    Column {
        id: column
        CheckBox {
            text: qsTr("First")
            ButtonGroup.group: bg
        }
        CheckBox {
            text: qsTr("Second")
            ButtonGroup.group: bg
        }
        CheckBox {
            text: qsTr("Third")
            ButtonGroup.group: bg
        }
    }

    Column {
        x: 100
        ButtonGroup {
            id: childGroup
            exclusive: false
            checkState: parentBox.checkState
        }

        CheckBox {
            id: parentBox
            text: qsTr("父按钮")
            tristate: true
            checkState: childGroup.checkState
            nextCheckState: () => {
                switch (parentBox.checkState) {
                case Qt.Unchecked:          // 未选中
                    return Qt.Checked;
                case Qt.PartiallyChecked:   // 部分选中
                    return Qt.Unchecked;
                case Qt.Checked:            // 完全选中
                    return Qt.PartiallyChecked;
                }
                return Qt.Unchecked;
            }
        }

        CheckBox {
            checked: true
            text: qsTr("儿子 1")
            leftPadding: indicator.width
            ButtonGroup.group: childGroup
        }

        CheckBox {
            text: qsTr("儿子 2")
            leftPadding: indicator.width
            ButtonGroup.group: childGroup
        }
    }

    DelayButton {
        y: 100
        width: 150
        height: 50

        delay: 3000 // 延迟的时间

        // 进度条
        onProgressChanged: {
            console.log("ProgressChanged", progress);
        }

        // 达到 1.0 时候会触发这个
        onActivated: {
            console.log("点击完成!");
        }
    }

    Column {
        x: 200
        y: 100
        RadioButton {
            checked: true
            text: qsTr("First")
        }
        RadioButton {
            text: qsTr("Second")
        }
        RadioButton {
            text: qsTr("Third")
        }
    }

    Column {
        x: 300
        y: 100
        Switch {
            text: qsTr("Wi-Fi")
            LayoutMirroring.enabled: true // 镜像
            onPositionChanged: {
                console.log("pos:", position);
            }
            onVisualPositionChanged: {
                console.log("V-Pos:", visualPosition);
            }
        }
        Switch {
            text: qsTr("Bluetooth")
        }
    }

    TabBar {
        x: 400
        y: 100
        id: bar
        width: 200
        TabButton {
            text: qsTr("Home")
        }
        TabButton {
            text: qsTr("Discover")
        }
        TabButton {
            text: qsTr("Activity")
        }
    }

    RoundButton {
        x: 100
        y: 200
        text: "圆角按钮"
        radius: 45 // 就是多了这个属性, 从 `background` 中提出来的
    }

    Button {
        id: btn
        x: 200
        y: 200
        width: 100; height: 50;
        text: "文本"
        contentItem: Rectangle {
            color: "transparent"
            Text {
                id: txt
                text: btn.text
                anchors.verticalCenter: parent.verticalCenter
                font.family: "黑体"
            }
            Image {
                id: img
                anchors.left: txt.right
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                source: "qrc:/img/misaka.png"
                width: 50; height: 50;
            }
        }
    }
}