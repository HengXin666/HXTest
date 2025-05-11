#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include <QObject>
#include <qml/HxData.h>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    // 强制使用 Basic 样式
    QQuickStyle::setStyle("Basic");
    QQmlApplicationEngine engine;

    // 设置自定义的全局的qml的上下文对象
    QQmlContext* cp = engine.rootContext();
    cp->setContextProperty("hx_width", 300);
    // cp->setContextProperty("HxData", HxData::get());

    // 注册一个C++自定义对象到 qml 中 (需要实例化为组件, 然后通过id使用)
    qmlRegisterType<HxData>(
        "HXDataObj",    // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0,           // 主版本号 与 次版本号
        "HxData"        // qml中使用的组件名称 (注意得是大写开头)
    );

    // 注册一个C++单例到 qml 中 (直接通过 组件名称.属性 即可使用)
    qmlRegisterSingletonInstance(
        "HXDataObj",        // 导入时候的名称 (import Xxx) (注意得是大写开头)
        1, 0,               // 主版本号 与 次版本号
        "HxDataSingleton",  // qml中使用的组件名称 (注意得是大写开头)
        HxData::get()       // 单例对象指针
    );

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );
    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX_01_QML", "HX_16_CppAndQml");

    if constexpr (false) {
        // engine load 加载后, 从C++中将槽绑定到qml自定义信号
        auto list = engine.rootObjects();
        auto window = list.first();
        QObject::connect(window, SIGNAL(qmlHxSignal(QString, int)), 
                         HxData::get(), SLOT(cppSlots(QString, int)));
    } else if constexpr (false) {
        auto list = engine.rootObjects();
        auto window = list.first();
        QObject::connect(HxData::get(), SIGNAL(cppSignal(QVariant, int)), 
                         window, SLOT(qmlHxFunc(QVariant, int)));
    } else if constexpr (1) {
        // engine load 加载后, C++ 中使用qml的自定义函数
        auto list = engine.rootObjects();
        auto window = list.first();
        int res, arg_2 = 114514;
        QVariant arg_1 = "C++ 调用 Qml Func";
        QMetaObject::invokeMethod(
            window, "qmlHxMyFunc",
            Q_RETURN_ARG(int, res),
            Q_ARG(QVariant, arg_1),
            Q_ARG(int, arg_2)
        );
        qDebug() << res;
    }

    return app.exec();
}
