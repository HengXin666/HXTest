#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);
    // 强制使用 Basic 样式
    QQuickStyle::setStyle("Basic");
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection
    );
    // 应该使用 _ 和 [0-9a-Z], 不能使用`-`
    engine.loadFromModule("HX_01_QML", "HX_15_Signal");

    return app.exec();
}
