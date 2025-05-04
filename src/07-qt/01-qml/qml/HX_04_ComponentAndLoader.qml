import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("04: Hello World")

    // 自定义组件
    Component {
        id: com
        Rectangle {
            id: rect
            width: 200
            height: 100
            color: "#2b2b2b"
            Component.onCompleted: {
                console.log("Component: onCompleted");
            }
            Component.onDestruction: {
                console.log("Component: ~onDestruction");
            }
        }
    }

    Component {
        id: imgCom
        AnimatedImage {
            id: gif
            source: "qrc:/img/maye.gif"
            width: 100
            height: 100
        }

        // Image {
        //     id: img
        //     source: "qrc:/img/misaka.png"
        //     // 或者写为 source: "/img/misaka.png", 反正都是从 qrc 中找...
        // }
    }

    Loader {
        id: comLoader
        sourceComponent: imgCom
        asynchronous: true
        onStatusChanged: {
            console.log("StateChanged:", status);
        }
    }

    Button {
        x: 250
        width: 50
        height: 50
        onClicked: {
            comLoader.item.paused = !comLoader.item.paused;

            // // com.rect.width = 100;
            // comLoader.item.width = 50;
            // comLoader.item.height = 50;
            // comLoader.sourceComponent = null;
        }
    }

    // Loader {
    //     // 从相对路径 ./ui/MyRectangle.qml 查找
    //     // 特别的, 如果是 /ui/MyRectangle.qml 那么是从 qrc 路径中查找
    //     source: "ui/MyRectangle.qml"
    //     width: 100
    //     height: 100
    // }

    Component.onCompleted: { // 组件生命周期回调函数
                             // onCompleted 会在创建完成后, 调用
        console.log("onCompleted", width, height, title);
    }
    Component.onDestruction: { // onDestruction 会在销毁完成后, 调用
        console.log("~onDestruction");
    }
}