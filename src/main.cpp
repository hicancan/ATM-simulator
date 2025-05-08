// main.cpp
/**
 * @file main.cpp
 * @brief 应用程序入口文件
 *
 * 程序的起点，负责创建 QApplication、QQmlApplicationEngine、AppController
 * 并加载 QML 文件。
 */
#include <QGuiApplication> // 包含 QGuiApplication 头文件
#include <QQmlApplicationEngine> // 包含 QQmlApplicationEngine 头文件
#include <QQmlContext> // 包含 QQmlContext 头文件
#include <QtQuickControls2/QQuickStyle> // 包含 QQuickStyle 头文件
#include <QDir> // 包含 QDir 头文件
#include <QStandardPaths> // 包含 QStandardPaths 头文件

// 包含应用程序控制器的头文件
#include "AppController.h"

/**
 * @brief 应用程序主函数
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @return 应用程序退出码
 */
int main(int argc, char *argv[])
{
    // 设置应用程序的组织名称、领域和名称
    QCoreApplication::setOrganizationName("ATMSimulator");
    QCoreApplication::setOrganizationDomain("example.com"); // 示例领域，请根据实际情况修改
    QCoreApplication::setApplicationName("ATM Simulator");

    // 创建 QGuiApplication 实例
    QGuiApplication app(argc, argv);

    // 设置 Qt Quick Controls 2 的样式
    QQuickStyle::setStyle("Material"); // 使用 Material 风格

    // 创建 QQmlApplicationEngine 实例
    QQmlApplicationEngine engine;

    // 添加 QML 导入路径，确保 QML 引擎能够找到自定义组件和模块
    engine.addImportPath(":/qml");
    engine.addImportPath(":/qml/components");

    // 创建并初始化 AppController
    // AppController 负责 Model/ViewModel 的生命周期管理和信号连接
    AppController controller;
    controller.initialize(&engine); // 初始化控制器，例如注册 QML 类型

    // 将 AppController 实例设置为 QML 上下文属性，使其在 QML 中可访问
    engine.rootContext()->setContextProperty("controller", &controller);

    // 加载主 QML 文件
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        // 检查 QML 文件是否成功加载
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1); // 加载失败则退出应用程序
    }, Qt::QueuedConnection); // 使用 QueuedConnection 确保对象创建完成后执行

    engine.load(url); // 加载主 QML 文件

    // 运行应用程序事件循环
    return app.exec();
}