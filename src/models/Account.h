/**
 * @file Account.h
 * @brief 账户数据实体类
 *
 * 定义了账户的属性和相关方法，包括序列化和反序列化功能。
 */
#pragma once

#include <QString>
#include <QJsonObject>
#include <QDateTime>
#include <QCryptographicHash>

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

    // 常量定义
    static const int MAX_FAILED_ATTEMPTS = 3; //!< 最大登录失败次数
    static const int TEMP_LOCK_DURATION = 15; //!< 临时锁定时长（分钟）

    // 属性
    QString cardNumber;     //!< 卡号，唯一标识账户
    QString pinHash;        //!< PIN 码的哈希值，用于身份验证
    QString salt;           //!< 盐值，用于增强哈希安全性
    QString holderName;     //!< 持卡人姓名
    double balance;         //!< 当前账户余额
    double withdrawLimit;   //!< 单次取款限额
    bool isLocked;          //!< 账户是否被锁定
    bool isAdmin;           //!< 是否为管理员账户
    int failedLoginAttempts; //!< 连续登录失败次数
    QDateTime lastFailedLogin; //!< 最后一次登录失败时间
    QDateTime temporaryLockTime; //!< 临时锁定到期时间

    /**
     * @brief 检查账户是否有效
     * @return 如果账户数据有效返回 true
     */
    bool isValid() const;

    /**
     * @brief 检查卡号是否有效
     * @param cardNumber 要验证的卡号
     * @return 如果卡号格式有效返回 true
     */
    static bool isValidCardNumber(const QString& cardNumber);
    
    /**
     * @brief 检查当前账户卡号是否有效
     * @return 如果卡号格式有效返回 true
     */
    bool isValidCardNumber() const;

    /**
     * @brief 检查PIN是否有效
     * @param pin PIN码
     * @return 如果PIN格式有效返回 true
     */
    static bool isValidPin(const QString& pin);
    
    /**
     * @brief 检查PIN是否匹配
     * @param pin 要验证的PIN码
     * @return 如果PIN码匹配返回 true
     */
    bool verifyPin(const QString& pin) const;
    
    /**
     * @brief 设置PIN码（会自动哈希）
     * @param pin 新的PIN码
     */
    void setPin(const QString& pin);
    
    /**
     * @brief 生成PIN码的哈希值
     * @param pin PIN码
     * @param salt 盐值
     * @return 哈希值
     */
    static QString hashPin(const QString& pin, const QString& salt);
    
    /**
     * @brief 生成随机盐值
     * @return 盐值字符串
     */
    static QString generateSalt();
    
    /**
     * @brief 记录登录失败
     * @return 是否触发了临时锁定
     */
    bool recordFailedLogin();
    
    /**
     * @brief 重置登录失败次数
     */
    void resetFailedLoginAttempts();
    
    /**
     * @brief 检查是否临时锁定
     * @return 如果账户被临时锁定返回 true
     */
    bool isTemporarilyLocked() const;

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