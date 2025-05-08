/**
 * @file Account.cpp
 * @brief 账户数据实体类实现
 *
 * 实现了Account类中定义的方法。
 */
#include "Account.h"
#include <QDebug>

/**
 * @brief 带参数的构造函数
 */
Account::Account(const QString& cardNumber, const QString& pin, const QString& holderName,
        double balance, double withdrawLimit, bool isLocked, bool isAdmin)
    : cardNumber(cardNumber)
    , pin(pin)
    , holderName(holderName)
    , balance(balance)
    , withdrawLimit(withdrawLimit)
    , isLocked(isLocked)
    , isAdmin(isAdmin)
{
}

/**
 * @brief 检查账户是否有效
 * @return 如果账户数据有效返回 true
 */
bool Account::isValid() const
{
    // 卡号必须为16位数字
    if (cardNumber.length() != 16) {
        return false;
    }
    
    for (const QChar &c : cardNumber) {
        if (!c.isDigit()) {
            return false;
        }
    }
    
    // PIN必须有效
    if (!isValidPin()) {
        return false;
    }
    
    // 持卡人姓名不能为空
    if (holderName.isEmpty()) {
        return false;
    }
    
    // 余额和取款限额必须为非负数
    if (balance < 0 || withdrawLimit < 0) {
        return false;
    }
    
    return true;
}

/**
 * @brief 检查PIN是否有效
 * @return 如果PIN格式有效返回 true
 */
bool Account::isValidPin() const
{
    // PIN必须为4-6位数字
    if (pin.length() < 4 || pin.length() > 6) {
        return false;
    }
    
    for (const QChar &c : pin) {
        if (!c.isDigit()) {
            return false;
        }
    }
    
    return true;
}

/**
 * @brief 将 Account 对象转换为 QJsonObject
 * @return 包含账户数据的 QJsonObject
 */
QJsonObject Account::toJson() const
{
    QJsonObject json;
    json["cardNumber"] = cardNumber;
    json["pin"] = pin;
    json["holderName"] = holderName;
    json["balance"] = balance;
    json["withdrawLimit"] = withdrawLimit;
    json["isLocked"] = isLocked;
    json["isAdmin"] = isAdmin;
    return json;
}

/**
 * @brief 从 QJsonObject 创建 Account 对象
 * @param json 包含账户数据的 QJsonObject
 * @return 创建的 Account 对象
 */
Account Account::fromJson(const QJsonObject &json)
{
    Account account;
    account.cardNumber = json["cardNumber"].toString();
    account.pin = json["pin"].toString();
    account.holderName = json["holderName"].toString();
    account.balance = json["balance"].toDouble();
    account.withdrawLimit = json["withdrawLimit"].toDouble();
    account.isLocked = json["isLocked"].toBool();
    // 兼容旧数据，如果 isAdmin 字段不存在，默认为 false
    account.isAdmin = json.contains("isAdmin") ? json["isAdmin"].toBool() : false;
    return account;
} 