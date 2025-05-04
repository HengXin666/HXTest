import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Basic

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("06: Hello World")
    Button {
        id: btn
        width: 50; height: 50;
        // onClicked: { btn.checked = !btn.checked }
        onPressed: {}
        onReleased: {}
        onPressAndHold: {}
        onDoubleClicked: {}

        // flat: true // 使按钮为背景色 (只有点击的时候才会显现)
        // highlighted: true

        checkable: true
        autoExclusive: true

        // checked: true
        // onCheckedChanged: console.log("checked", checked)
    }

    Button {
        width: 50; height: 50;
        x: 60
        autoExclusive: true
        checkable: true

        // Pressed只会在按下的时候触发, Down你鼠标移开也会触发
        onDownChanged: console.log("down:", down, "pressed:", pressed)
        onPressed: console.log("!onPressed")

        // icon.source: "qrc:/img/misaka.png"
        // icon.color: "red"

        indicator: Image {
            anchors.fill: parent
            source: "qrc:/img/misaka.png"
        }

        text: "文本"
    }

    Button {
        id: btn3
        width: 50; height: 50;
        x: 120
        autoExclusive: true
        checkable: true

        // 长按多次触发
        autoRepeat: true
        autoRepeatDelay: 3000       // 控制长按到触发所需的时间 (ms)
        autoRepeatInterval: 1000    // 控制触发的时间间隔 (ms)

        onClicked: console.log("onClicked")
        onPressed: console.log("onPressed")
        onReleased: console.log("onReleased")

        background: Rectangle {
            anchors.fill: btn3
            color: btn3.pressed ? "#990099" : "#2b2b2b"
            border.color: btn3.pressed ? "#2b2b2b" : "#990099"
        }
    }
}