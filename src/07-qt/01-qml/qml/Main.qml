import QtQuick
import QtQuick.Controls

Window { // root 控件, 父窗口是主界面
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    property int hxVal: 0

    onHxValChanged: {
        console.log("hxVal 被修改了", hxVal);
    }

    color: "#990099"

    // 相对于 父控件 的位置
    x: 50
    y: 50

    // 控制窗口大小
    minimumWidth: 400
    minimumHeight: 300
    maximumWidth: 720
    maximumHeight: 560

    // 设置窗口透明度: 0.00 ~ 1.00
    opacity: 0.5

    // 宽度变化触发该槽
    onWidthChanged: {
        console.log("宽度变化:", width)
    }

    // 焦点
    Button {
        id: btn1
        objectName: "左边的按钮"
        focus: true // 设置默认获取焦点
        
        x: 50
        y: 50
        width: 50
        height: 50

        background: Rectangle { // 设置矩形边框样式
            border.width: 6
            // 根据是否有焦点, 响应式更新
            border.color: btn1.focus ? "blue" : "black";
        }

        onCheckedChanged: {
            console.log("点击了按钮")
        }
        Keys.onRightPressed: {
            btn2.focus = true;
        }
    }

    Button {
        id: btn2
        objectName: "右边的按钮"
        focus: true
        
        x: 150
        y: 50
        width: 50
        height: 50

        background: Rectangle {
            border.width: 6
            border.color: btn2.focus ? "blue" : "black";
        }

        onCheckedChanged: {
            console.log("点击了按钮")
        }
        Keys.onLeftPressed: {
            btn1.focus = true;
        }
    }

    // 焦点改变信号
    onActiveFocusItemChanged: {
        console.log("当前焦点所在元素", activeFocusItem, activeFocusItem.objectName);
    }
}
