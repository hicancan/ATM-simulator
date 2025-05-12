// JsonAccountRepository.cpp
/**
 * @file JsonAccountRepository.cpp
 * @brief JSON账户存储库类实现
 *
 * 实现了JsonAccountRepository类中定义的文件操作和账户管理方法。
 */
#include "JsonAccountRepository.h"
#include <QDebug>

/**
 * @brief 默认构造函数
 *
 * 创建自己的JSON持久化管理器实例，使用默认的"accounts.json"文件。
 */
JsonAccountRepository::JsonAccountRepository() 
    : m_filename("accounts.json")
    , m_persistenceManager(new JsonPersistenceManager())
    , m_isDirty(false)
    , m_ownsPersistenceManager(true)
{
    // 尝试从文件加载账户数据
    // 如果加载失败，则初始化测试账户并保存到文件
    if (!loadAccounts()) {
        qDebug() << "无法加载账户数据，初始化测试账户";
        initializeTestAccounts();
        saveAccounts(); // 保存初始化的测试账户
    }
}

/**
 * @brief 构造函数
 * @param persistenceManager JSON持久化管理器
 * @param filename 账户数据文件名
 */
JsonAccountRepository::JsonAccountRepository(JsonPersistenceManager* persistenceManager, 
                                           const QString& filename)
    : m_filename(filename)
    , m_persistenceManager(persistenceManager)
    , m_isDirty(false)
    , m_ownsPersistenceManager(false)
{
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
 *
 * 在对象销毁时保存账户数据，确保不会丢失内存中的更改。
 * 注意：大多数操作如saveAccount()和deleteAccount()已经主动调用了saveAccounts()，
 * 这里作为最后的保障措施，防止意外情况下数据未保存。
 */
JsonAccountRepository::~JsonAccountRepository()
{
    // 仅当数据被修改时才保存
    if (m_isDirty) {
        saveAccounts();
    }
    
    // 如果持有持久化管理器的所有权，则释放它
    if (m_ownsPersistenceManager && m_persistenceManager) {
        delete m_persistenceManager;
        m_persistenceManager = nullptr;
    }
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
    m_isDirty = true;
    
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
    m_isDirty = true;
    
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
    QJsonArray accountsArray;

    // 将所有账户转换为 JSON 数组
    for (const auto &account : m_accounts) {
        accountsArray.append(account.toJson());
    }

    // 使用持久化管理器保存数据
    bool success = m_persistenceManager->saveToFile(m_filename, accountsArray);
    if (success) {
        m_isDirty = false;
        qDebug() << "成功保存" << m_accounts.size() << "个账户";
    }
    
    return success;
}

/**
 * @brief 从文件加载所有账户数据
 * @return 如果成功加载返回true，否则返回false
 */
bool JsonAccountRepository::loadAccounts()
{
    QJsonArray accountsArray;
    
    // 使用持久化管理器加载数据
    if (!m_persistenceManager->loadFromFile(m_filename, accountsArray)) {
        return false;
    }

    // 清空当前账户列表
    m_accounts.clear();

    // 从 JSON 数组加载账户数据
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
        admin.holderName = "管理员";
        admin.balance = 50000.0;
        admin.withdrawLimit = 10000.0;
        admin.isLocked = false;
        admin.isAdmin = true;
        // 设置PIN码（自动哈希）
        admin.setPin("8888");
        m_accounts[admin.cardNumber] = admin;
        m_isDirty = true;
    }

    qDebug() << "成功加载" << m_accounts.size() << "个账户";
    return true;
}

/**
 * @brief 添加账户到内存映射
 * @param account 要添加的账户
 */
void JsonAccountRepository::addAccount(const Account& account)
{
    m_accounts[account.cardNumber] = account;
    m_isDirty = true;
}

/**
 * @brief 初始化测试账户数据
 *
 * 如果数据文件不存在，则创建一些预设的测试账户。
 */
void JsonAccountRepository::initializeTestAccounts()
{
    // 添加普通测试账户（使用构造函数将自动哈希PIN）
    Account account1("1234567890123456", "1234", "张三", 50000.0, 20000.0, false, false);
    addAccount(account1);

    Account account2("2345678901234567", "2345", "李四", 100000.0, 30000.0, false, false);
    addAccount(account2);

    Account account3("3456789012345678", "3456", "王五", 75000.0, 25000.0, true, false);
    addAccount(account3);

    // 添加管理员账户
    Account adminAccount("9999888877776666", "8888", "管理员", 500000.0, 100000.0, false, true);
    addAccount(adminAccount);

    qDebug() << "测试账户初始化完成，共" << m_accounts.size() << "个账户";
} 