import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("11: Hello World")

    Repeater {
        model: 3
        delegate: Button {
            width: 50; height: 50;
            y: index * 60
        }
    }

    Repeater {
        model: ["文本1", "文本2", "我是文本三"]
        delegate: Button {
            width: 50; height: 50;
            x: 100
            y: index * 60
            text: modelData
        }
    }
}