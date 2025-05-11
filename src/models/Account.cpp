/**
 * @file Account.cpp
 * @brief 账户数据实体类实现
 *
 * 实现了Account类中定义的方法。
 */
#include "Account.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QDateTime>

/**
 * @brief 带参数的构造函数
 */
Account::Account(const QString& cardNumber, const QString& pin, const QString& holderName,
        double balance, double withdrawLimit, bool isLocked, bool isAdmin)
    : cardNumber(cardNumber)
    , holderName(holderName)
    , balance(balance)
    , withdrawLimit(withdrawLimit)
    , isLocked(isLocked)
    , isAdmin(isAdmin)
    , failedLoginAttempts(0)
{
    // 生成盐值并哈希PIN码
    salt = generateSalt();
    pinHash = hashPin(pin, salt);
}

/**
 * @brief 检查卡号是否有效
 * @param cardNumber 要验证的卡号
 * @return 如果卡号格式有效返回 true
 */
bool Account::isValidCardNumber(const QString& cardNumber)
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
    
    return true;
}

/**
 * @brief 检查当前账户卡号是否有效
 * @return 如果卡号格式有效返回 true
 */
bool Account::isValidCardNumber() const
{
    // 使用静态方法验证当前账户卡号
    return isValidCardNumber(cardNumber);
}

/**
 * @brief 检查账户是否有效
 * @return 如果账户数据有效返回 true
 */
bool Account::isValid() const
{
    // 验证卡号格式
    if (!isValidCardNumber()) {
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
 * @param pin PIN码
 * @return 如果PIN格式有效返回 true
 */
bool Account::isValidPin(const QString& pin)
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
 * @brief 生成随机盐值
 * @return 盐值字符串
 */
QString Account::generateSalt()
{
    const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int saltLength = 16;
    QString salt;
    
    for (int i = 0; i < saltLength; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        salt.append(chars.at(index));
    }
    
    return salt;
}

/**
 * @brief 生成PIN码的哈希值
 * @param pin PIN码
 * @param salt 盐值
 * @return 哈希值
 */
QString Account::hashPin(const QString& pin, const QString& salt)
{
    QByteArray pinData = (pin + salt).toUtf8();
    QByteArray hashedPin = QCryptographicHash::hash(pinData, QCryptographicHash::Sha256).toHex();
    return QString::fromUtf8(hashedPin);
}

/**
 * @brief 检查PIN是否匹配
 * @param pin 要验证的PIN码
 * @return 如果PIN码匹配返回 true
 */
bool Account::verifyPin(const QString& pin) const
{
    // 生成输入PIN的哈希值
    QString inputHash = hashPin(pin, salt);
    
    // 比较哈希值
    return (inputHash == pinHash);
}

/**
 * @brief 设置PIN码（会自动哈希）
 * @param pin 新的PIN码
 */
void Account::setPin(const QString& pin)
{
    if (isValidPin(pin)) {
        salt = generateSalt();
        pinHash = hashPin(pin, salt);
    }
}

/**
 * @brief 记录登录失败
 * @return 是否触发了临时锁定
 */
bool Account::recordFailedLogin()
{
    failedLoginAttempts++;
    lastFailedLogin = QDateTime::currentDateTime();
    
    // 检查是否需要临时锁定
    if (failedLoginAttempts >= MAX_FAILED_ATTEMPTS) {
        temporaryLockTime = lastFailedLogin.addSecs(TEMP_LOCK_DURATION * 60);
        qDebug() << "账户" << cardNumber << "因连续登录失败被临时锁定，锁定至" 
                 << temporaryLockTime.toString();
        return true;
    }
    
    return false;
}

/**
 * @brief 重置登录失败次数
 */
void Account::resetFailedLoginAttempts()
{
    failedLoginAttempts = 0;
    temporaryLockTime = QDateTime();
}

/**
 * @brief 检查是否临时锁定
 * @return 如果账户被临时锁定返回 true
 */
bool Account::isTemporarilyLocked() const
{
    if (!temporaryLockTime.isValid()) {
        return false;
    }
    
    // 检查临时锁定是否已过期
    return QDateTime::currentDateTime() < temporaryLockTime;
}

/**
 * @brief 将 Account 对象转换为 QJsonObject
 * @return 包含账户数据的 QJsonObject
 */
QJsonObject Account::toJson() const
{
    QJsonObject json;
    json["cardNumber"] = cardNumber;
    json["pinHash"] = pinHash;
    json["salt"] = salt;
    json["holderName"] = holderName;
    json["balance"] = balance;
    json["withdrawLimit"] = withdrawLimit;
    json["isLocked"] = isLocked;
    json["isAdmin"] = isAdmin;
    json["failedLoginAttempts"] = failedLoginAttempts;
    
    if (lastFailedLogin.isValid()) {
        json["lastFailedLogin"] = lastFailedLogin.toString(Qt::ISODate);
    }
    
    if (temporaryLockTime.isValid()) {
        json["temporaryLockTime"] = temporaryLockTime.toString(Qt::ISODate);
    }
    
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
    
    // 处理新旧两种存储格式
    if (json.contains("pinHash") && json.contains("salt")) {
        // 新格式：使用哈希PIN
        account.pinHash = json["pinHash"].toString();
        account.salt = json["salt"].toString();
    } else if (json.contains("pin")) {
        // 旧格式：使用明文PIN，需要转换为哈希格式
        QString plainPin = json["pin"].toString();
        account.salt = generateSalt();
        account.pinHash = hashPin(plainPin, account.salt);
    }
    
    account.holderName = json["holderName"].toString();
    account.balance = json["balance"].toDouble();
    account.withdrawLimit = json["withdrawLimit"].toDouble();
    account.isLocked = json["isLocked"].toBool();
    // 兼容旧数据，如果 isAdmin 字段不存在，默认为 false
    account.isAdmin = json.contains("isAdmin") ? json["isAdmin"].toBool() : false;
    
    // 加载登录失败相关数据
    account.failedLoginAttempts = json.contains("failedLoginAttempts") ? 
                                 json["failedLoginAttempts"].toInt() : 0;
    
    if (json.contains("lastFailedLogin")) {
        account.lastFailedLogin = QDateTime::fromString(json["lastFailedLogin"].toString(), Qt::ISODate);
    }
    
    if (json.contains("temporaryLockTime")) {
        account.temporaryLockTime = QDateTime::fromString(json["temporaryLockTime"].toString(), Qt::ISODate);
    }
    
    return account;
} 