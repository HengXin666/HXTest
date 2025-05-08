import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("12: Hello World")

    ListView {
        width: 180; height: 200
        spacing: 10
        model: ["按钮1", "按钮2", "按钮3"] // 数字 / list, 控制了所有的数据
        delegate: Button { // 委托: item, 控制每一项数据是如何绘制的
            required property string modelData
            text: modelData
        }
    }

    ListModel {
        id: listModel
        ListElement {
            // 可以自定义元素键值对
            hxName: "Bill Smith"
            number: "555 3264"
        }
        ListElement {
            hxName: "John Brown"
            number: "555 8426"
        }
        ListElement {
            hxName: "Sam Wise"
            number: "555 0473"
        }
    }

    ListView {
        x: 200
        id: list
        width: 180; height: 200
        spacing: 10
        model: listModel
        // 页眉
        header: Rectangle {
            width: parent.width; height: 30;
            color: "#990099" 
        }
        // 页脚
        footer: Rectangle {
            width: parent.width; height: 30;
            color: "#2b2b2b" 
        }
        highlight: Rectangle { // 高亮选中的项
            color: "lightsteelblue"
            radius: 5
        }
        delegate: Rectangle {
            id: rect
            width: 180; height: 30;
            required property int index
            required property string hxName
            required property string number
            property var listRef: ListView.view // 注意! `ListView.view`只能在顶层访问
                                                // 内层的 MouseArea 嵌套是无法访问的! (会是null)

            color: "transparent" // 需要原本的内容是透明的
            Text {
                anchors.centerIn: parent
                text: rect.hxName + ": " + rect.number
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    // 特别的, list 的 delegate 和外部是隔离的(因为它是在Componen中动态实例化的),
                    // 不一定能访问到外部控件的id
                    // 因此跟现代的做法是使用 ListView.view 类似于 loaderId.item 的做法
                    console.log("idx:", rect.listRef);    // 当前选中的项
                    rect.listRef.currentIndex = rect.index;          // 当前项索引 (index)
                }
            }
        }
    }

    ListView {
        x: 400
        width: 180; height: 200
        spacing: 10
        model: ListModel {
            ListElement {
                hxSize: "标题一"
                name: "Cat"
                section: "喵喵喵"
            }
            ListElement {
                hxSize: "标题也得有"
                name: "Dog"
                section: "哇哇哇"
            }
            ListElement {
                hxSize: "标题也得有"
                name: "吗喽"
                section: "呀~糊"
            }
            ListElement {
                hxSize: "标题二"
                name: "Cat"
                section: "xaxa"
            }
            ListElement {
                hxSize: "标题也得有"
                name: "Dog"
                section: "嗷嗷嗷"
            }
            ListElement {
                hxSize: "标题也得有"
                name: "吗喽"
                section: "芜湖"
            }
        }
        delegate: Text {
            required property string name
            text: name
            font.pixelSize: 18
        }
        section.property: "hxSize"
        section.criteria: ViewSection.FullString
        section.delegate: Rectangle {
            width: ListView.view.width
            height: 30
            color: "lightsteelblue"

            required property string section

            Text {
                text: parent.section
                font.bold: true
                font.pixelSize: 20
            }
        }
    }
}