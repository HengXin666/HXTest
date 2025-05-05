import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("07: Hello World")

    Component {
        id: com
        Rectangle {
            id: root
            property int _valInt: 0
            property double _valDouble: 0
            property real _valReal: -1e-10 // 平台相关浮点数 (一般是 float)
            property string _str: "#990099"
            property color _color: "#990099"
            property url _url: "https://hengxin666.github.io/HXLoLi/"

            // 支持任意类型 (不推荐滥用)
            property variant _variant: [1, "1"]
            property var _var: [1, "1"]

            // 列表 (如果类型不对, 那么会产生强转, 强转失败, 则用默认值)
            property list<int> _list: [1, 1.1, 2, "3", [1, 2, 3]]

            // 属性别名
            property alias _rectColor: rect.color

            // 只读属性
            readonly property int constexprBufSize: 1024

            width: 50
            height: 50
            color: _color // 可以使用`string`类型的 _str 来赋值, 但是规范上来说, 最好还是使用正确的类型

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                onClicked: {
                    console.log(parent._list);
                }
            }

            Rectangle {
                id: rect
                width: 25; height: 25;
            }
        }
    }

    Loader {
        sourceComponent: com
        onLoaded: {
            item._rectColor = "#2b2b2b";
            // item.constexprBufSize = 1;
            // 报错: qrc:/HX_01_QML/qml/HX_07_Property.qml:56: TypeError: Cannot assign to read-only property "constexprBufSize"
            console.log("const:", item.constexprBufSize);
        }
    }
}