import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    width: 100; height: 100;
    signal hxButtonClicked(string msg);
    Button {
        id: btn
        signal hxButtonNaKa(string msg); // 这个外面收不到, 你喊什么都妹用!
        text: "按钮1"
        onClicked: {
            root.hxButtonClicked("偷偷告诉你, 这个是自定义信号: 按钮1被人按了!");
            hxButtonNaKa("别听它乱说, 我才是按钮!");
        }
    }
}