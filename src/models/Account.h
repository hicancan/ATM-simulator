/**
 * @file Account.h
 * @brief 账户数据实体类
 *
 * 定义了账户的属性和相关方法，包括序列化和反序列化功能。
 */
#pragma once

#include <QString>
#include <QJsonObject>

/**
 * @brief 账户数据类
 *
 * 存储单个账户的详细信息及序列化方法。
 */
class Account {
public:
    /**
     * @brief 默认构造函数
     */
    Account() = default;
    
    /**
     * @brief 带参数的构造函数
     */
    Account(const QString& cardNumber, const QString& pin, const QString& holderName,
            double balance, double withdrawLimit, bool isLocked = false, bool isAdmin = false);

    // 属性
    QString cardNumber;     //!< 卡号，唯一标识账户
    QString pin;            //!< PIN 码，用于身份验证
    QString holderName;     //!< 持卡人姓名
    double balance;         //!< 当前账户余额
    double withdrawLimit;   //!< 单次取款限额
    bool isLocked;          //!< 账户是否被锁定
    bool isAdmin;           //!< 是否为管理员账户

    /**
     * @brief 检查账户是否有效
     * @return 如果账户数据有效返回 true
     */
    bool isValid() const;

    /**
     * @brief 检查PIN是否有效
     * @return 如果PIN格式有效返回 true
     */
    bool isValidPin() const;

    /**
     * @brief 将 Account 对象转换为 QJsonObject
     * @return 包含账户数据的 QJsonObject
     */
    QJsonObject toJson() const;

    /**
     * @brief 从 QJsonObject 创建 Account 对象
     * @param json 包含账户数据的 QJsonObject
     * @return 创建的 Account 对象
     */
    static Account fromJson(const QJsonObject &json);
}; 