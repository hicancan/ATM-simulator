/**
 * @file JsonPersistenceManager.h
 * @brief JSON持久化管理器头文件
 *
 * 负责处理JSON格式数据的文件读写操作，集中管理所有持久化逻辑。
 */
#pragma once

#include <QString>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QObject>
#include <functional>

/**
 * @brief JSON持久化管理器
 *
 * 负责处理JSON格式数据的文件读写操作，集中管理所有持久化逻辑。
 */
class JsonPersistenceManager : public QObject {
    Q_OBJECT
    
public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     * @param dataPath 数据存储路径，默认使用应用程序的本地数据目录
     */
    explicit JsonPersistenceManager(QObject* parent = nullptr, const QString& dataPath = QString());

    /**
     * @brief 保存JSON数组到文件
     * @param filename 文件名
     * @param jsonArray 要保存的JSON数组
     * @return 如果成功保存返回true，否则返回false
     */
    bool saveToFile(const QString& filename, const QJsonArray& jsonArray);

    /**
     * @brief 从文件加载JSON数组
     * @param filename 文件名
     * @param jsonArray 输出参数，保存读取的JSON数组
     * @return 如果成功加载返回true，否则返回false
     */
    bool loadFromFile(const QString& filename, QJsonArray& jsonArray);

    /**
     * @brief 获取数据存储路径
     * @return 数据存储路径
     */
    QString getDataPath() const;

private:
    //!< 数据存储路径
    QString m_dataPath;
}; 