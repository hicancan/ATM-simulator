// JsonAccountRepository.cpp
/**
 * @file JsonAccountRepository.cpp
 * @brief JSON账户存储库类实现
 *
 * 实现了JsonAccountRepository类中定义的文件操作和账户管理方法。
 */
#include "JsonAccountRepository.h"
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>

/**
 * @brief 构造函数
 * @param dataPath 数据存储路径
 * @param filename 账户数据文件名
 */
JsonAccountRepository::JsonAccountRepository(const QString& dataPath, const QString& filename)
    : m_filename(filename)
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

    qDebug() << "账户数据存储路径:" << m_dataPath;

    // 尝试从文件加载账户数据
    // 如果加载失败，则初始化测试账户并保存到文件
    if (!loadAccounts()) {
        qDebug() << "无法加载账户数据，初始化测试账户";
        initializeTestAccounts();
        saveAccounts(); // 保存初始化的测试账户
    }
}

/**
 * @brief 析构函数
 */
JsonAccountRepository::~JsonAccountRepository()
{
    // 在析构函数中保存账户数据
    saveAccounts();
}

/**
 * @brief 保存单个账户
 * @param account 要保存的账户
 * @return 操作结果
 */
OperationResult JsonAccountRepository::saveAccount(const Account& account)
{
    // 检查账户是否有效
    if (!account.isValid()) {
        return OperationResult::Failure("账户数据无效");
    }
    
    // 添加或更新账户到内存映射
    m_accounts[account.cardNumber] = account;
    
    // 保存所有账户数据到文件
    if (!saveAccounts()) {
        return OperationResult::Failure("无法保存账户数据");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 删除账户
 * @param cardNumber 要删除的账户卡号
 * @return 操作结果
 */
OperationResult JsonAccountRepository::deleteAccount(const QString& cardNumber)
{
    // 检查账户是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 从内存映射中移除账户
    m_accounts.remove(cardNumber);
    
    // 保存所有账户数据到文件
    if (!saveAccounts()) {
        return OperationResult::Failure("无法保存账户数据");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 根据卡号查找账户
 * @param cardNumber 卡号
 * @return 包含账户的optional对象，如果未找到则为empty
 */
std::optional<Account> JsonAccountRepository::findByCardNumber(const QString& cardNumber) const
{
    auto it = m_accounts.find(cardNumber);
    if (it != m_accounts.end()) {
        return it.value();
    }
    return std::nullopt;
}

/**
 * @brief 获取所有账户
 * @return 所有账户的列表
 */
QVector<Account> JsonAccountRepository::getAllAccounts() const
{
    QVector<Account> accounts;
    for (const auto& account : m_accounts) {
        accounts.append(account);
    }
    return accounts;
}

/**
 * @brief 检查账户是否存在
 * @param cardNumber 卡号
 * @return 如果账户存在返回true，否则返回false
 */
bool JsonAccountRepository::accountExists(const QString& cardNumber) const
{
    return m_accounts.contains(cardNumber);
}

/**
 * @brief 保存所有账户数据到文件
 * @return 如果成功保存返回true，否则返回false
 */
bool JsonAccountRepository::saveAccounts()
{
    QString filePath = m_dataPath + "/" + m_filename;
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存账户数据:" << filePath << ", 错误:" << file.errorString();
        return false;
    }

    QJsonArray accountsArray;

    // 将所有账户转换为 JSON 数组
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

/**
 * @brief 从文件加载所有账户数据
 * @return 如果成功加载返回true，否则返回false
 */
bool JsonAccountRepository::loadAccounts()
{
    QString filePath = m_dataPath + "/" + m_filename;
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

    // 从 JSON 数组加载账户数据
    QJsonArray accountsArray = doc.array();
    for (const QJsonValue &value : accountsArray) {
        if (value.isObject()) {
            Account account = Account::fromJson(value.toObject());
            m_accounts[account.cardNumber] = account;
        }
    }

    // 确保管理员账户加载正确或重新创建
    if (!accountExists("9999888877776666")) {
        qWarning() << "管理员账户未加载，创建新管理员账户";
        Account admin;
        admin.cardNumber = "9999888877776666";
        admin.pin = "8888";
        admin.holderName = "管理员";
        admin.balance = 50000.0;
        admin.withdrawLimit = 10000.0;
        admin.isLocked = false;
        admin.isAdmin = true;
        m_accounts[admin.cardNumber] = admin;
    }

    qDebug() << "成功从" << filePath << "加载" << m_accounts.size() << "个账户";
    return true;
}

/**
 * @brief 添加账户到内存映射
 * @param account 要添加的账户
 */
void JsonAccountRepository::addAccount(const Account& account)
{
    m_accounts[account.cardNumber] = account;
}

/**
 * @brief 初始化测试账户数据
 *
 * 如果数据文件不存在，则创建一些预设的测试账户。
 */
void JsonAccountRepository::initializeTestAccounts()
{
    // 添加普通测试账户
    Account account1("1234567890123456", "1234", "张三", 5000.0, 2000.0, false, false);
    addAccount(account1);

    Account account2("2345678901234567", "2345", "李四", 10000.0, 3000.0, false, false);
    addAccount(account2);

    Account account3("3456789012345678", "3456", "王五", 7500.0, 2500.0, true, false);
    addAccount(account3);

    // 添加管理员账户
    Account adminAccount("9999888877776666", "8888", "管理员", 50000.0, 10000.0, false, true);
    addAccount(adminAccount);

    qDebug() << "测试账户初始化完成，共" << m_accounts.size() << "个账户";
} 