#include "AccountModel.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

AccountModel::AccountModel(QObject *parent)
    : QObject(parent)
{
    // 设置数据存储路径
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(m_dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    qDebug() << "账户数据存储路径:" << m_dataPath;
    
    // 尝试加载账户数据，如果加载失败则初始化测试账户
    if (!loadAccounts()) {
        qDebug() << "无法加载账户数据，初始化测试账户";
        initializeTestAccounts();
        saveAccounts(); // 保存初始化的测试账户
    }
}

AccountModel::~AccountModel()
{
    // 在析构函数中保存账户数据
    saveAccounts();
}

bool AccountModel::validateCredentials(const QString &cardNumber, const QString &pin)
{
    const Account* account = findAccount(cardNumber);
    if (!account) {
        return false;  // Card not found
    }
    
    if (account->isLocked) {
        return false;  // Account is locked
    }
    
    return account->pin == pin;
}

bool AccountModel::withdrawAmount(const QString &cardNumber, double amount)
{
    Account* account = findAccount(cardNumber);
    if (!account || account->isLocked) {
        return false;
    }
    
    if (amount <= 0 || amount > account->withdrawLimit) {
        return false;  // Invalid amount or exceeds limit
    }
    
    if (account->balance < amount) {
        return false;  // Insufficient funds
    }
    
    account->balance -= amount;
    saveAccounts(); // 操作后保存数据
    return true;
}

bool AccountModel::depositAmount(const QString &cardNumber, double amount)
{
    Account* account = findAccount(cardNumber);
    if (!account || account->isLocked) {
        return false;
    }
    
    if (amount <= 0) {
        return false;  // Invalid amount
    }
    
    account->balance += amount;
    saveAccounts(); // 操作后保存数据
    return true;
}

bool AccountModel::transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    // 检查卡号不能相同
    if (fromCardNumber == toCardNumber) {
        return false;  // 不能转账给自己
    }
    
    // 获取两个账户
    Account* fromAccount = findAccount(fromCardNumber);
    Account* toAccount = findAccount(toCardNumber);
    
    // 检查账户是否存在且未锁定
    if (!fromAccount || fromAccount->isLocked) {
        return false;  // 源账户不存在或已锁定
    }
    
    if (!toAccount) {
        return false;  // 目标账户不存在
    }
    
    // 检查转账金额
    if (amount <= 0) {
        return false;  // 无效金额
    }
    
    if (fromAccount->balance < amount) {
        return false;  // 余额不足
    }
    
    // 执行转账
    fromAccount->balance -= amount;
    toAccount->balance += amount;
    
    saveAccounts(); // 操作后保存数据
    return true;
}

bool AccountModel::accountExists(const QString &cardNumber) const
{
    return findAccount(cardNumber) != nullptr;
}

QString AccountModel::getAccountHolderName(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->holderName : QString();
}

double AccountModel::getBalance(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->balance : 0.0;
}

QString AccountModel::getHolderName(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->holderName : QString();
}

double AccountModel::getWithdrawLimit(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->withdrawLimit : 0.0;
}

bool AccountModel::isAccountLocked(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->isLocked : true;
}

bool AccountModel::changePin(const QString &cardNumber, const QString &oldPin, const QString &newPin)
{
    Account* account = findAccount(cardNumber);
    
    // Check if account exists and is not locked
    if (!account || account->isLocked) {
        return false;
    }
    
    // Verify old PIN
    if (account->pin != oldPin) {
        return false; // Old PIN is incorrect
    }
    
    // Validate new PIN (should be 4 digits)
    if (newPin.length() != 4 || !newPin.toInt()) {
        return false; // Invalid new PIN
    }
    
    // Update PIN
    account->pin = newPin;
    saveAccounts(); // 操作后保存数据
    return true;
}

void AccountModel::lockAccount(const QString &cardNumber, bool locked)
{
    Account* account = findAccount(cardNumber);
    if (account) {
        account->isLocked = locked;
    }
}

void AccountModel::addAccount(const Account &account)
{
    m_accounts[account.cardNumber] = account;
}

void AccountModel::initializeTestAccounts()
{
    // Add some test accounts
    Account account1;
    account1.cardNumber = "1234567890123456";
    account1.pin = "1234";
    account1.holderName = "张三";
    account1.balance = 5000.0;
    account1.withdrawLimit = 2000.0;
    account1.isLocked = false;
    addAccount(account1);
    
    Account account2;
    account2.cardNumber = "2345678901234567";
    account2.pin = "2345";
    account2.holderName = "李四";
    account2.balance = 10000.0;
    account2.withdrawLimit = 3000.0;
    account2.isLocked = false;
    addAccount(account2);
    
    Account account3;
    account3.cardNumber = "3456789012345678";
    account3.pin = "3456";
    account3.holderName = "王五";
    account3.balance = 7500.0;
    account3.withdrawLimit = 2500.0;
    account3.isLocked = true;  // This account is locked
    addAccount(account3);
}

Account* AccountModel::findAccount(const QString &cardNumber)
{
    auto it = m_accounts.find(cardNumber);
    return (it != m_accounts.end()) ? &it.value() : nullptr;
}

const Account* AccountModel::findAccount(const QString &cardNumber) const
{
    auto it = m_accounts.find(cardNumber);
    return (it != m_accounts.end()) ? &it.value() : nullptr;
}

// 实现保存账户数据的方法
bool AccountModel::saveAccounts(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存账户数据:" << filePath << ", 错误:" << file.errorString();
        return false;
    }
    
    QJsonArray accountsArray;
    
    // 将所有账户转换为JSON数组
    for (const auto &account : m_accounts) {
        accountsArray.append(account.toJson());
    }
    
    QJsonDocument doc(accountsArray);
    
    // 写入文件
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "成功保存" << m_accounts.size() << "个账户到" << filePath;
    return true;
}

// 实现加载账户数据的方法
bool AccountModel::loadAccounts(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);
    
    if (!file.exists()) {
        qWarning() << "账户数据文件不存在:" << filePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件加载账户数据:" << filePath << ", 错误:" << file.errorString();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "账户数据文件格式无效:" << filePath;
        return false;
    }
    
    // 清空当前账户列表
    m_accounts.clear();
    
    // 加载账户数据
    QJsonArray accountsArray = doc.array();
    for (const QJsonValue &value : accountsArray) {
        if (value.isObject()) {
            Account account = Account::fromJson(value.toObject());
            m_accounts[account.cardNumber] = account;
        }
    }
    
    qDebug() << "成功从" << filePath << "加载" << m_accounts.size() << "个账户";
    return true;
} 