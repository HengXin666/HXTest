import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("13: Hello World")

    ComboBox {
        textRole: "hxText" // 作为文本的变量名称 (ListElement 中)
        valueRole: "hxVal" // 作为值的变量名称
        // 自定义显示文本
        displayText: `${currentText} -> ${currentValue}`
        model: ListModel {
            id: hxListModel
            ListElement {
                hxText: "文本1"
                hxVal: 123
            }
            ListElement {
                hxText: "文本2"
                hxVal: 456
            }
            ListElement {
                hxText: "文本3"
                hxVal: 789
            }
        }
        onCurrentTextChanged: { // 当前选择的项对应的文本
            console.log("txt:", currentText);
        }
        onCurrentValueChanged: { // 当前选择的项对应的值
            console.log("val:", currentValue);
        }
    }

    ComboBox {
        x: 300
        editable: true
        // validator: IntValidator { // 匹配数字, 不怎么好用
        //     top: 100
        //     bottom: 0
        // }
        validator: RegularExpressionValidator { // 仅可输入正则表达式: (16进制数)
            regularExpression: /0x[0-9A-F]+/
        }
        onAcceptableInputChanged: { // 匹配状态改变时候触发
            console.log("匹配状态:", acceptableInput);
        }

        model: ListModel {
            id: model
            ListElement {
                text: "选项1"
            }
            ListElement {
                text: "选项2"
            }
            ListElement {
                text: "选项3"
            }
        }
        Component.onCompleted: {
            console.log("cnt:", count);
        }
        onAccepted: {
            if (find(editText) === -1) {
                model.append({text: editText});
            }
        }
    }

    ComboBox {
        y: 200
        id: control
        model: ["选项1", "选项2", "选项3"]

        // 自定义绘制 框项
        contentItem: Text {
            leftPadding: 0
            rightPadding: control.indicator.width + control.spacing

            text: control.displayText
            font: control.font
            color: control.pressed ? "red" : "blue" // 是否鼠标按下它, 而改变颜色
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        // 自定义每一项是如何绘制的
        delegate: ItemDelegate {
            id: itDg
            property int index;
            property string modelData;
            property ComboBox control;

            width: control.width
            contentItem: Text {
                text: itDg.modelData
                color: "#990099"
                font: itDg.control.font
                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
            }
            highlighted: control.highlightedIndex === index
        }
    }
}