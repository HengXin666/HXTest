import QtQuick

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("09: Hello World")

    Rectangle {
        x: 50; y: 50;
        width: 50; height: 50;
        border.color: "#000000"

        Text {
            id: txt
            anchors.fill: parent
            text: qsTr("zzzzz\nLoli")
            wrapMode: Text.WordWrap

            Component.onCompleted: {
                console.log("width", contentWidth);
                console.log("height", contentHeight);
                console.log("行数:", lineCount);
                console.log("每一行字体间隔的高度", lineHeight)
            }

            font.family: "萝莉体"
            font.italic: true       // 斜体
            font.letterSpacing: 10  // 字母之间的横向距离
            font.pixelSize: 16      // 字体大小: px (像素)
            // font.pointSize: 16      // 字体大小: 磅
            font.underline: true    // 下划线
        }
    }

    Column {
        x: 200
        Text {
            font.pointSize: 24
            text: "<b>Hello</b> <i>World!</i>"
        }
        Text {
            font.pointSize: 24
            textFormat: Text.RichText
            text: "<b>Hello</b> <i>World!</i>"
        }
        Text {
            font.pointSize: 24
            textFormat: Text.PlainText
            text: "<b>Hello</b> <i>World!</i>"
        }
        Text {
            font.pointSize: 24
            textFormat: Text.MarkdownText
            text: "**Hello** *World!*"
        }
    }

    Text {
        y: 300
        font.pixelSize: 16
        textFormat: Text.MarkdownText
        text: "See the [Qt Project website](http://qt-project.org) or [HXLoLi](https://hengxin666.github.io/HXLoLi/docs/)"
        onLinkActivated: (link) => {
            console.log("点击了链接:", link);
        }
        // 进出链接会触发
        onLinkHovered: (link) => {
            console.log("选停在链接上:", link);
        }
        // 表示当前鼠标悬停处的链接 (hoveredLink), 改变时候, 就会触发 (实际上, 和上面的没有什么区别)
        // 大概率是因为这个是属性, 自动生成的 onXxxChanged 导致的, 可能会出现一些是一样的.
        // 虽然他们本质语义是不同的: 一个是`信号`, 一个是`属性变更通知`
        onHoveredLinkChanged: {
            console.log("onHoveredLinkChanged", hoveredLink);
        }

        // 如果希望悬浮在url上, 有光标样式改变, 可以像下面这样
        // 不过坏处是, 点击的话, 需要自定义编写, 因为鼠标信号被 MouseArea 捕获了, 不会透过...
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: parent.hoveredLink !== "" ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: {
                const link = parent.hoveredLink;
                if (link !== "") {
                    console.log("[onClicked] 点击了链接:", link);
                }
            }
        }
    }
}