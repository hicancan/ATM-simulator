/**
 * @file JsonPersistenceManager.cpp
 * @brief JSON持久化管理器实现文件
 *
 * 实现了JsonPersistenceManager类中定义的文件操作和JSON管理方法。
 */
#include "JsonPersistenceManager.h"
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QDebug>

JsonPersistenceManager::JsonPersistenceManager(QObject* parent, const QString& dataPath)
    : QObject(parent)
{
    // 如果未提供数据路径，则使用应用程序的本地数据目录
    if (dataPath.isEmpty()) {
        m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    } else {
        m_dataPath = dataPath;
    }
    
    QDir dir(m_dataPath);
    // 如果目录不存在，则创建它
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    qDebug() << "数据存储路径:" << m_dataPath;
}

bool JsonPersistenceManager::saveToFile(const QString& filename, const QJsonArray& jsonArray)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存数据:" << filePath << ", 错误:" << file.errorString();
        return false;
    }

    QJsonDocument doc(jsonArray);

    // 写入文件
    file.write(doc.toJson());
    file.close();

    qDebug() << "成功保存数据到" << filePath;
    return true;
}

bool JsonPersistenceManager::loadFromFile(const QString& filename, QJsonArray& jsonArray)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);

    if (!file.exists()) {
        qWarning() << "数据文件不存在:" << filePath;
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件加载数据:" << filePath << ", 错误:" << file.errorString();
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "数据文件格式无效:" << filePath;
        return false;
    }

    jsonArray = doc.array();
    qDebug() << "成功从" << filePath << "加载数据";
    return true;
}

QString JsonPersistenceManager::getDataPath() const
{
    return m_dataPath;
} 