import QtQuick
import QtQuick.Controls
import HXDataObj 1.0

pragma ComponentBehavior: Bound

Window {
    id: root
    width: hx_width
    height: 480
    visible: true
    title: qsTr("16: Hello World")

    signal qmlHxSignal(string str, int v);

    function qmlHxFunc(str, v: int) {
        console.log("[qmlHxFunc]: ", str, v);
    }

    function qmlHxMyFunc(str, v: int): int {
        console.log("[qmlHxMyFunc]: ", str, v);
        return v % 10;
    }

    Button {
        onClicked: {
            root.qmlHxSignal("wc", 666);
            // hxData.cppSignal("C++", 520);
            // HxDataSingleton.fun();
            // hxData.fun(); // 没有触发连接到的槽 (因为根本不是它连接)
            console.log("hxData 实例化对象:", hxData._val);
            console.log("HxDataSingleton 单例:", HxDataSingleton._val);
        }
    }

    HxData {
        id: hxData
        _val: 10
        _str: "我是字符串"
        Component.onCompleted: {
            _val += 10;
            console.log(_val, _str);
            fun();
        }
    }

    // === 从qml中将c++槽绑定到qml自定义信号 === {
    // // 方法一:
    // Connections {
    //     target: root
    //     function onQmlHxSignal(str, x) {
    //         hxData.cppSlots(str, x);
    //     }
    // }

    // // 方法二:
    // Component.onCompleted: {
    //     qmlHxSignal.connect(hxData.cppSlots);
    // }
    // } === 从qml中将c++槽绑定到qml自定义信号 ===

    // === 从qml中将qml槽绑定到c++自定义信号 === {
    // 方法一:
    // Connections {
    //     target: hxData
    //     function onCppSignal(str: string, x: int) {
    //         root.qmlHxFunc(str, x);
    //     }
    // }

    // 方法二:
    // Component.onCompleted: {
    //     HxDataSingleton.cppSignal.connect(qmlHxFunc);
    // }
    // } === 从qml中将qml槽绑定到c++自定义信号 ===
}