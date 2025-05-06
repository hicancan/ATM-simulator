#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QtQuickControls2/QQuickStyle>
#include <QDir>

#include "AppController.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("ATMSimulator");
    app.setOrganizationDomain("example.com");
    app.setApplicationName("ATM Simulator");
    
    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;
    
    // 添加导入路径，确保能找到自定义组件
    engine.addImportPath(":/qml");
    engine.addImportPath(":/qml/components");
    
    // Create and initialize the controller
    AppController controller;
    controller.initialize(&engine);
    
    // Make the controller available in QML
    engine.rootContext()->setContextProperty("controller", &controller);
    
    // Load the main QML file
    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
} 