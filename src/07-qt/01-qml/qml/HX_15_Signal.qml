import QtQuick
import QtQuick.Controls
import "ui"

pragma ComponentBehavior: Bound

Window {
    id: root
    width: 640
    height: 480
    visible: true
    title: qsTr("15: Hello World")

    signal hxMySignal(string str, int sum); // 自定义信号

    function func(ss, ii) { // 函数
        console.log("[func]:", ss, ii);
    }

    // 自动生成的槽 (默认和`hxMySignal`信号连接了)
    onHxMySignal: {
        console.log("[HX]: siganl");
    }

    Button {
        text: "自定义槽"
        onClicked: {
            root.hxMySignal("abc", 123); // 发送信号
        }
    }

    Component.onCompleted: {
        hxMySignal.connect(func); // 给信号绑定槽
    }

    Connections {
        target: root
        function onHxMySignal(str, val) {
            console.log("[Connections]:", str, val);
        }
    }

    Loader {
        id: loader
        y: 200
        sourceComponent: MySignalRect {
            Connections {
                target: loader.item
                ignoreUnknownSignals: true  // 连接到不存在的信号会产生运行时错误
                                            // 设置为 true 表示忽略这些错误警告
                function onHxButtonClicked(msg: string) {
                    console.log("里面说:", msg);
                    console.log("我回复道:", "徒弟我来救你啦");
                }
            }
            onHxButtonClicked: (msg) => {
                console.log("(真没办法) 里面说:", msg);
            }
            // Connections {
            //     target: loader.item.btn // 届かない
            //     function onHxButtonNaKa(msg: string) {
            //         console.log("里面说:", msg);
            //         console.log("我回复道:", "我徒弟呢!?");
            //     }
            // }
        }
    }
}