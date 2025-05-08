// AccountModel.cpp
/**
 * @file AccountModel.cpp
 * @brief 账户数据模型实现文件
 *
 * 实现了 AccountModel 类中定义的账户数据管理、验证和业务逻辑方法。
 * 负责与持久化存储交互（模拟 JSON 文件存储）并执行账户操作。
 */
#include "AccountModel.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <numeric> // 用于 std::accumulate
#include <algorithm> // 用于 std::min

/**
 * @brief 构造函数
 * @param parent 父对象
 */
AccountModel::AccountModel(QObject *parent)
    : QObject(parent)
    , m_transactionModel(nullptr) //!< 初始化交易模型指针为空
{
    // 设置数据存储路径到应用程序的本地数据目录。
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(m_dataPath);
    // 如果目录不存在，则创建它。
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    qDebug() << "账户数据存储路径:" << m_dataPath;

    // 尝试从文件加载账户数据。
    // 如果加载失败，则初始化测试账户并保存到文件。
    if (!loadAccounts()) {
        qDebug() << "无法加载账户数据，初始化测试账户";
        initializeTestAccounts();
        saveAccounts(); // 保存初始化的测试账户
    }
}

/**
 * @brief 析构函数
 *
 * 在对象销毁时保存账户数据。
 */
AccountModel::~AccountModel()
{
    // 在析构函数中保存账户数据
    saveAccounts();
}

/**
 * @brief 设置交易数据模型引用
 * @param transactionModel 交易数据模型指针
 */
void AccountModel::setTransactionModel(TransactionModel* transactionModel)
{
    m_transactionModel = transactionModel;
}

/**
 * @brief 记录交易
 *
 * 调用关联的 TransactionModel 记录交易信息。
 *
 * @param cardNumber 交易涉及的卡号
 * @param type 交易类型
 * @param amount 交易金额
 * @param balanceAfter 交易后余额
 * @param description 交易描述
 * @param targetCard 目标卡号 (转账时使用)
 */
void AccountModel::recordTransaction(const QString &cardNumber, TransactionType type,
                                     double amount, double balanceAfter,
                                     const QString &description, const QString &targetCard)
{
    // 检查 TransactionModel 是否可用。
    if (!m_transactionModel) {
        qWarning() << "交易记录失败：未设置 TransactionModel";
        return;
    }

    // 检查卡号是否合法。
    if (!accountExists(cardNumber)) {
        qWarning() << "交易记录失败：卡号" << cardNumber << "不存在";
        return;
    }

    // 使用 TransactionModel 记录交易。
    m_transactionModel->recordTransaction(cardNumber, type, amount, balanceAfter, description, targetCard);

    qDebug() << "成功记录交易，卡号:" << cardNumber
             << "，类型:" << static_cast<int>(type)
             << "，金额:" << amount
             << "，交易后余额:" << balanceAfter;
}

/**
 * @brief 验证账户凭据
 *
 * 检查卡号和 PIN 是否匹配且账户未被锁定。
 *
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateCredentials(const QString &cardNumber, const QString &pin)
{
    const Account* account = findAccount(cardNumber);
    if (!account) {
        qDebug() << "验证失败: 卡号不存在:" << cardNumber;
        return OperationResult::Failure("卡号不存在");
    }

    if (account->isLocked) {
        qDebug() << "验证失败: 账户已锁定:" << cardNumber;
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }

    bool pinMatches = (account->pin == pin);
    qDebug() << "PIN验证结果:" << pinMatches << "输入PIN:" << pin << "存储PIN:" << account->pin;
    
    if (!pinMatches) {
        return OperationResult::Failure("PIN码不正确");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 执行取款金额操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::withdrawAmount(const QString &cardNumber, double amount)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号不存在");
    }
    
    if (account->isLocked) {
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }

    if (amount <= 0) {
        return OperationResult::Failure("取款金额必须大于0");
    }
    
    if (amount > account->withdrawLimit) {
        return OperationResult::Failure(QString("超出单次取款限额 ￥%1").arg(account->withdrawLimit));
    }

    if (account->balance < amount) {
        return OperationResult::Failure("余额不足");
    }

    account->balance -= amount;
    saveAccounts(); // 操作后保存数据
    return OperationResult::Success();
}

/**
 * @brief 执行存款金额操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::depositAmount(const QString &cardNumber, double amount)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号不存在");
    }
    
    if (account->isLocked) {
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }

    if (amount <= 0) {
        return OperationResult::Failure("存款金额必须大于0");
    }

    account->balance += amount;
    saveAccounts(); // 操作后保存数据
    return OperationResult::Success();
}

/**
 * @brief 执行转账金额操作
 * @param fromCardNumber 源卡号
 * @param toCardNumber 目标卡号
 * @param amount 转账金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    // 检查卡号不能相同
    if (fromCardNumber == toCardNumber) {
        return OperationResult::Failure("不能转账给自己");
    }

    // 获取源账户和目标账户
    Account* fromAccount = findAccount(fromCardNumber);
    Account* toAccount = findAccount(toCardNumber);

    // 检查账户是否存在且源账户未锁定
    if (!fromAccount) {
        return OperationResult::Failure("源账户不存在");
    }
    
    if (fromAccount->isLocked) {
        return OperationResult::Failure("您的账户已被锁定，请联系管理员");
    }

    if (!toAccount) {
        return OperationResult::Failure("目标账户不存在");
    }
    
    if (toAccount->isLocked) {
        return OperationResult::Failure("目标账户已被锁定，无法转账");
    }

    // 检查转账金额
    if (amount <= 0) {
        return OperationResult::Failure("转账金额必须大于0");
    }

    if (fromAccount->balance < amount) {
        return OperationResult::Failure("余额不足");
    }

    // 执行转账
    fromAccount->balance -= amount;
    toAccount->balance += amount;

    saveAccounts(); // 操作后保存数据
    return OperationResult::Success();
}

/**
 * @brief 检查账户是否存在
 * @param cardNumber 卡号
 * @return 如果账户存在返回 true，否则返回 false
 */
bool AccountModel::accountExists(const QString &cardNumber) const
{
    return findAccount(cardNumber) != nullptr;
}

/**
 * @brief 获取账户持卡人姓名
 * @param cardNumber 卡号
 * @return 持卡人姓名，如果账户不存在返回空字符串
 */
QString AccountModel::getAccountHolderName(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->holderName : QString();
}

/**
 * @brief 获取账户余额
 * @param cardNumber 卡号
 * @return 账户余额，如果账户不存在返回 0.0
 */
double AccountModel::getBalance(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->balance : 0.0;
}

/**
 * @brief 获取账户持卡人姓名
 * @param cardNumber 卡号
 * @return 持卡人姓名，如果账户不存在返回空字符串
 */
QString AccountModel::getHolderName(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->holderName : QString();
}

/**
 * @brief 获取账户取款限额
 * @param cardNumber 卡号
 * @return 取款限额，如果账户不存在返回 0.0
 */
double AccountModel::getWithdrawLimit(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->withdrawLimit : 0.0;
}

/**
 * @brief 检查账户是否被锁定
 * @param cardNumber 卡号
 * @return 如果账户被锁定返回 true，否则返回 false
 */
bool AccountModel::isAccountLocked(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->isLocked : false;
}

/**
 * @brief 修改账户 PIN 码
 * @param cardNumber 卡号
 * @param oldPin 旧 PIN 码
 * @param newPin 新 PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::changePin(const QString &cardNumber, const QString &oldPin, const QString &newPin)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号不存在");
    }

    if (account->isLocked) {
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }

    if (account->pin != oldPin) {
        return OperationResult::Failure("当前PIN码不正确");
    }

    if (newPin.length() < 4 || newPin.length() > 6) {
        return OperationResult::Failure("新PIN码长度必须在4-6位之间");
    }

    // 检查PIN是否为纯数字
    bool isNumeric = true;
    for (QChar c : newPin) {
        if (!c.isDigit()) {
            isNumeric = false;
            break;
        }
    }
    
    if (!isNumeric) {
        return OperationResult::Failure("PIN码只能包含数字");
    }

    account->pin = newPin;
    saveAccounts();
    return OperationResult::Success();
}

/**
 * @brief 设置账户锁定状态
 * @param cardNumber 卡号
 * @param locked 是否锁定
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号不存在");
    }

    account->isLocked = locked;
    saveAccounts();
    
    QString statusMessage = locked ? "账户已锁定" : "账户已解锁";
    qDebug() << statusMessage << "卡号:" << cardNumber;
    
    return OperationResult::Success();
}

/**
 * @brief 添加账户到内存列表
 * @param account 要添加的 Account 对象
 */
void AccountModel::addAccount(const Account &account)
{
    m_accounts[account.cardNumber] = account;
}

/**
 * @brief 初始化测试账户数据
 *
 * 如果数据文件不存在，则创建一些预设的测试账户。
 */
void AccountModel::initializeTestAccounts()
{
    // 添加一些测试账户
    Account account1;
    account1.cardNumber = "1234567890123456";
    account1.pin = "1234";
    account1.holderName = "张三";
    account1.balance = 5000.0;
    account1.withdrawLimit = 2000.0;
    account1.isLocked = false;
    account1.isAdmin = false;
    addAccount(account1);

    Account account2;
    account2.cardNumber = "2345678901234567";
    account2.pin = "2345";
    account2.holderName = "李四";
    account2.balance = 10000.0;
    account2.withdrawLimit = 3000.0;
    account2.isLocked = false;
    account2.isAdmin = false;
    addAccount(account2);

    Account account3;
    account3.cardNumber = "3456789012345678";
    account3.pin = "3456";
    account3.holderName = "王五";
    account3.balance = 7500.0;
    account3.withdrawLimit = 2500.0;
    account3.isLocked = true;  // 此账户被锁定
    account3.isAdmin = false;
    addAccount(account3);

    // 添加管理员账户
    Account adminAccount;
    adminAccount.cardNumber = "9999888877776666";
    adminAccount.pin = "8888"; // 确保管理员 PIN 码是字符串
    adminAccount.holderName = "管理员";
    adminAccount.balance = 50000.0;
    adminAccount.withdrawLimit = 10000.0;
    adminAccount.isLocked = false;
    adminAccount.isAdmin = true; // 设置为管理员

    qDebug() << "管理员账户初始化 PIN 码:" << adminAccount.pin;
    addAccount(adminAccount);

    // 再次检查管理员账户是否正确保存
    const Account* savedAdmin = findAccount("9999888877776666");
    if (savedAdmin) {
        qDebug() << "管理员账户保存后 PIN 码:" << savedAdmin->pin;
    } else {
        qWarning() << "警告: 管理员账户保存失败!";
    }
}

/**
 * @brief 根据卡号查找账户（可修改版本）
 * @param cardNumber 卡号
 * @return 账户指针，如果未找到返回 nullptr
 */
Account* AccountModel::findAccount(const QString &cardNumber)
{
    auto it = m_accounts.find(cardNumber);
    return (it != m_accounts.end()) ? &it.value() : nullptr;
}

/**
 * @brief 根据卡号查找账户（常量版本）
 * @param cardNumber 卡号
 * @return 账户指针，如果未找到返回 nullptr
 */
const Account* AccountModel::findAccount(const QString &cardNumber) const
{
    auto it = m_accounts.find(cardNumber);
    return (it != m_accounts.end()) ? &it.value() : nullptr;
}

/**
 * @brief 保存账户数据到文件
 * @param filename 文件名 (默认为 accounts.json)
 * @return 如果成功保存返回 true，否则返回 false
 */
bool AccountModel::saveAccounts(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
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
 * @brief 从文件加载账户数据
 * @param filename 文件名 (默认为 accounts.json)
 * @return 如果成功加载返回 true，否则返回 false
 */
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

    // 从 JSON 数组加载账户数据
    QJsonArray accountsArray = doc.array();
    for (const QJsonValue &value : accountsArray) {
        if (value.isObject()) {
            Account account = Account::fromJson(value.toObject());

            // 确保管理员 PIN 码正确
            if (account.cardNumber == "9999888877776666") {
                qDebug() << "加载管理员账户，PIN 码:" << account.pin;
                if (account.pin != "8888") {
                    qWarning() << "管理员 PIN 码不正确，重新设置为 8888";
                    account.pin = "8888";
                }
            }
            m_accounts[account.cardNumber] = account;
        }
    }

    // 确保管理员账户加载正确或重新创建
    const Account* adminAccount = findAccount("9999888877776666");
    if (adminAccount) {
        qDebug() << "管理员账户加载成功，PIN 码:" << adminAccount->pin;
    } else {
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
 * @brief 预测账户余额
 *
 * 根据历史交易记录预测未来一定天数后的余额。
 *
 * @param cardNumber 卡号
 * @param transactionModel 交易数据模型指针
 * @param daysInFuture 预测未来天数
 * @return 预测的余额，如果无法预测返回当前余额
 */
double AccountModel::predictBalance(const QString &cardNumber, const TransactionModel* transactionModel, int daysInFuture) const
{
    if (!transactionModel) {
        qWarning() << "TransactionModel 为空，无法预测余额。";
        return getBalance(cardNumber); // 如果没有交易模型，返回当前余额
    }

    QVector<Transaction> transactions = transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.size() < 2) {
        qWarning() << "交易记录不足，无法预测卡号为:" << cardNumber << " 的余额。";
        return getBalance(cardNumber); // 如果交易数据不足，返回当前余额
    }

    // 按日期排序交易记录（最新的在后）
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp < b.timestamp;
    });

    // 使用最近的 N 条交易记录进行趋势预测 (例如，最近 10 条或更少)
    int N = std::min(10, static_cast<int>(transactions.size()));
    if (N < 2) return getBalance(cardNumber); // 如果少于 2 条交易，无法计算变化

    QVector<double> balanceChanges;
    QVector<qint64> timeDiffsSeconds; // 时间差，单位秒

    // 计算最近 N-1 段交易的余额变化和时间差
    for (int i = transactions.size() - N + 1; i < transactions.size(); ++i) {
        balanceChanges.append(transactions[i].balanceAfter - transactions[i-1].balanceAfter);
        timeDiffsSeconds.append(transactions[i].timestamp.toSecsSinceEpoch() - transactions[i-1].timestamp.toSecsSinceEpoch());
    }

    if (balanceChanges.isEmpty() || timeDiffsSeconds.isEmpty()) {
        return getBalance(cardNumber); // 如果没有有效的时间段，返回当前余额
    }

    // 计算总余额变化和总时间差
    double totalBalanceChange = std::accumulate(balanceChanges.constBegin(), balanceChanges.constEnd(), 0.0);
    qint64 totalTimeDiffSeconds = std::accumulate(timeDiffsSeconds.constBegin(), timeDiffsSeconds.constEnd(), 0LL);

    if (totalTimeDiffSeconds == 0) {
        // 避免除以零，如果所有交易发生在同一秒（尽管不太可能）
        return transactions.last().balanceAfter;
    }

    // 计算每秒的平均余额变化
    double averageChangePerSecond = totalBalanceChange / totalTimeDiffSeconds;

    // 计算每日的平均余额变化
    double averageDailyChange = averageChangePerSecond * (24 * 60 * 60);

    // 计算预测余额
    double currentBalance = transactions.last().balanceAfter;
    double predictedBalance = currentBalance + (averageDailyChange * daysInFuture);

    qDebug() << "预测卡号为:" << cardNumber << " 的余额";
    qDebug() << "当前余额:" << currentBalance;
    qDebug() << "用于趋势计算的交易数量:" << N-1;
    qDebug() << "总余额变化:" << totalBalanceChange;
    qDebug() << "总时间差 (秒):" << totalTimeDiffSeconds;
    qDebug() << "平均每日变化:" << averageDailyChange;
    qDebug() << daysInFuture << " 天后的预测余额:" << predictedBalance;

    return predictedBalance;
}

/**
 * @brief 检查账户是否为管理员账户
 * @param cardNumber 卡号
 * @return 如果是管理员账户返回 true，否则返回 false
 */
bool AccountModel::isAdmin(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->isAdmin : false;
}

/**
 * @brief 获取所有账户列表
 * @return 包含所有 Account 结构体的 QVector
 */
QVector<Account> AccountModel::getAllAccounts() const
{
    QVector<Account> accounts;
    for (const auto &account : m_accounts) {
        accounts.append(account);
    }
    return accounts;
}

/**
 * @brief 创建账户
 * @param account 账户数据结构
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::createAccount(const Account &account)
{
    if (accountExists(account.cardNumber)) {
        return OperationResult::Failure("卡号已存在，不能创建重复账户");
    }

    // 验证PIN码格式
    if (account.pin.length() < 4 || account.pin.length() > 6) {
        return OperationResult::Failure("PIN码长度必须在4-6位之间");
    }

    // 检查PIN码是否为纯数字
    bool isNumeric = true;
    for (QChar c : account.pin) {
        if (!c.isDigit()) {
            isNumeric = false;
            break;
        }
    }
    
    if (!isNumeric) {
        return OperationResult::Failure("PIN码只能包含数字");
    }

    // 验证余额和取款限额
    if (account.balance < 0) {
        return OperationResult::Failure("账户余额不能为负数");
    }

    if (account.withdrawLimit <= 0) {
        return OperationResult::Failure("取款限额必须大于0");
    }

    addAccount(account);
    saveAccounts();
    qDebug() << "已创建新账户，卡号:" << account.cardNumber;
    
    return OperationResult::Success();
}

/**
 * @brief 更新账户
 * @param account 账户数据结构
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::updateAccount(const Account &account)
{
    Account* existingAccount = findAccount(account.cardNumber);
    if (!existingAccount) {
        return OperationResult::Failure("卡号不存在，无法更新");
    }
    
    // 验证余额和取款限额
    if (account.balance < 0) {
        return OperationResult::Failure("账户余额不能为负数");
    }

    if (account.withdrawLimit <= 0) {
        return OperationResult::Failure("取款限额必须大于0");
    }

    // 保留原始PIN和管理员状态，只更新其他字段
    existingAccount->holderName = account.holderName;
    existingAccount->balance = account.balance;
    existingAccount->withdrawLimit = account.withdrawLimit;
    existingAccount->isLocked = account.isLocked;
    
    saveAccounts();
    qDebug() << "已更新账户，卡号:" << account.cardNumber;
    
    return OperationResult::Success();
}

/**
 * @brief 删除账户
 * @param cardNumber 卡号
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::deleteAccount(const QString &cardNumber)
{
    // 检查账户是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("卡号不存在，无法删除");
    }

    // 获取账户以检查是否是管理员
    Account account = m_accounts.value(cardNumber);
    
    // 检查是否是最后一个管理员账户，如果是则不允许删除
    if (account.isAdmin) {
        int adminCount = 0;
        for (const auto& acc : m_accounts) {
            if (acc.isAdmin) {
                adminCount++;
            }
        }
        
        if (adminCount <= 1) {
            return OperationResult::Failure("不能删除系统中的最后一个管理员账户");
        }
    }

    // 从账户列表中移除(正确使用QMap的remove方法)
    m_accounts.remove(cardNumber);
    saveAccounts();
    
    // 如果设置了交易模型，还应清除相关交易记录
    if (m_transactionModel) {
        m_transactionModel->clearTransactionsForCard(cardNumber);
    }
    
    qDebug() << "已删除账户，卡号:" << cardNumber;
    
    return OperationResult::Success();
}

/**
 * @brief 设置账户取款限额
 * @param cardNumber 卡号
 * @param limit 取款限额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::setWithdrawLimit(const QString &cardNumber, double limit)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号不存在");
    }

    if (limit <= 0) {
        return OperationResult::Failure("取款限额必须大于0");
    }

    account->withdrawLimit = limit;
    saveAccounts();
    qDebug() << "已设置取款限额:" << limit << "卡号:" << cardNumber;
    
    return OperationResult::Success();
}

// --- 验证方法实现 ---

/**
 * @brief 验证登录操作
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateLogin(const QString &cardNumber, const QString &pin)
{
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请输入卡号");
    }

    if (pin.isEmpty()) {
        return OperationResult::Failure("请输入PIN码");
    }

    const Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("卡号或PIN码错误");
    }

    if (account->isLocked) {
        return OperationResult::Failure("账户已锁定，请联系银行客服");
    }

    if (account->pin != pin) {
        return OperationResult::Failure("卡号或PIN码错误");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证管理员登录
 * @param cardNumber 管理员卡号
 * @param pin 管理员 PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateAdminLogin(const QString &cardNumber, const QString &pin)
{
    // 特殊处理管理员账户验证
    if (cardNumber != "9999888877776666" || pin != "8888") {
        return OperationResult::Failure("管理员卡号或PIN码错误");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证取款操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateWithdrawal(const QString &cardNumber, double amount)
{
    const Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("账户不存在");
    }

    if (account->isLocked) {
        return OperationResult::Failure("账户已锁定");
    }

    if (amount <= 0) {
        return OperationResult::Failure("请输入有效金额");
    }

    if (amount > account->withdrawLimit) {
        return OperationResult::Failure(QString("超出取款限额，单笔最多可取￥%1").arg(account->withdrawLimit));
    }

    if (amount > account->balance) {
        return OperationResult::Failure("余额不足");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证存款操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateDeposit(const QString &cardNumber, double amount)
{
    const Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("账户不存在");
    }

    if (account->isLocked) {
        return OperationResult::Failure("账户已锁定");
    }

    if (amount <= 0) {
        return OperationResult::Failure("请输入有效金额");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证转账操作
 * @param fromCardNumber 源卡号
 * @param toCardNumber 目标卡号
 * @param amount 转账金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    if (toCardNumber.isEmpty()) {
        return OperationResult::Failure("请输入目标卡号");
    }

    if (fromCardNumber == toCardNumber) {
        return OperationResult::Failure("不能转账给自己");
    }

    const Account* fromAccount = findAccount(fromCardNumber);
    if (!fromAccount) {
        return OperationResult::Failure("源账户不存在");
    }

    if (fromAccount->isLocked) {
        return OperationResult::Failure("账户已锁定");
    }

    const Account* toAccount = findAccount(toCardNumber);
    if (!toAccount) {
        return OperationResult::Failure("目标账户不存在");
    }

    if (amount <= 0) {
        return OperationResult::Failure("请输入有效金额");
    }

    if (amount > fromAccount->balance) {
        return OperationResult::Failure("余额不足");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证 PIN 码修改操作
 * @param cardNumber 卡号
 * @param currentPin 当前 PIN 码
 * @param newPin 新 PIN 码
 * @param confirmPin 确认新 PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validatePinChange(const QString &cardNumber, const QString &currentPin, const QString &newPin, const QString &confirmPin)
{
    const Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("账户不存在");
    }

    if (account->isLocked) {
        return OperationResult::Failure("账户已锁定");
    }

    if (account->pin != currentPin) {
        return OperationResult::Failure("当前密码错误");
    }

    if (newPin.length() != 4 || !newPin.toInt()) {
        return OperationResult::Failure("新密码必须是4位数字");
    }

    if (newPin != confirmPin) {
        return OperationResult::Failure("确认密码与新密码不匹配");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证创建账户参数
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @param holderName 持卡人姓名
 * @param balance 初始余额
 * @param withdrawLimit 取款限额
 * @param isAdmin 是否为管理员账户
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateCreateAccount(const QString &cardNumber, const QString &pin, const QString &holderName,
                                              double balance, double withdrawLimit, bool isAdmin)
{
    if (cardNumber.length() != 16 || !cardNumber.toULongLong()) {
        return OperationResult::Failure("卡号必须是16位数字");
    }

    if (accountExists(cardNumber)) {
        return OperationResult::Failure("卡号已存在");
    }

    if (pin.length() != 4 || !pin.toInt()) {
        return OperationResult::Failure("PIN码必须是4位数字");
    }

    if (holderName.isEmpty()) {
        return OperationResult::Failure("持卡人姓名不能为空");
    }

    if (balance < 0) {
        return OperationResult::Failure("余额不能为负数");
    }

    if (withdrawLimit < 0) {
        return OperationResult::Failure("取款限额不能为负数");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证更新账户参数
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param balance 余额
 * @param withdrawLimit 取款限额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateUpdateAccount(const QString &cardNumber, const QString &holderName,
                                              double balance, double withdrawLimit)
{
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("卡号不存在");
    }

    if (holderName.isEmpty()) {
        return OperationResult::Failure("持卡人姓名不能为空");
    }

    if (balance < 0) {
        return OperationResult::Failure("余额不能为负数");
    }

    if (withdrawLimit < 0) {
        return OperationResult::Failure("取款限额不能为负数");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证目标账户（转账时使用）
 * @param sourceCard 源卡号
 * @param targetCard 目标卡号
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateTargetAccount(const QString &sourceCard, const QString &targetCard)
{
    // 检查源账户
    if (!accountExists(sourceCard)) {
        return OperationResult::Failure("源账户不存在");
    }

    // 检查目标账户
    if (!accountExists(targetCard)) {
        return OperationResult::Failure("目标账户不存在");
    }

    // 不能转账给自己
    if (sourceCard == targetCard) {
        return OperationResult::Failure("不能转账给自己");
    }

    // 检查目标账户是否被锁定
    if (isAccountLocked(targetCard)) {
        return OperationResult::Failure("目标账户已锁定，无法转账");
    }

    return OperationResult::Success();
}

/**
 * @brief 验证管理员操作权限
 * @param cardNumber 卡号
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::validateAdminOperation(const QString &cardNumber)
{
    // 验证卡号存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("卡号不存在");
    }

    // 验证是否为管理员
    if (!isAdmin(cardNumber)) {
        return OperationResult::Failure("没有管理员权限执行此操作");
    }

    return OperationResult::Success();
}


// --- 账户操作方法实现 ---

/**
 * @brief 执行取款操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::performWithdrawal(const QString &cardNumber, double amount)
{
    // 首先验证操作
    OperationResult result = validateWithdrawal(cardNumber, amount);
    if (!result.success) {
        return result;
    }

    // 执行取款
    Account* account = findAccount(cardNumber);
    account->balance -= amount;
    saveAccounts();

    return OperationResult::Success();
}

/**
 * @brief 执行存款操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::performDeposit(const QString &cardNumber, double amount)
{
    // 首先验证操作
    OperationResult result = validateDeposit(cardNumber, amount);
    if (!result.success) {
        return result;
    }

    // 执行存款
    Account* account = findAccount(cardNumber);
    account->balance += amount;
    saveAccounts();

    return OperationResult::Success();
}

/**
 * @brief 执行转账操作
 * @param fromCardNumber 源卡号
 * @param toCardNumber 目标卡号
 * @param amount 转账金额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::performTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    // 首先验证操作
    OperationResult result = validateTransfer(fromCardNumber, toCardNumber, amount);
    if (!result.success) {
        return result;
    }

    // 执行转账
    Account* fromAccount = findAccount(fromCardNumber);
    Account* toAccount = findAccount(toCardNumber);

    fromAccount->balance -= amount;
    toAccount->balance += amount;
    saveAccounts();

    return OperationResult::Success();
}

/**
 * @brief 执行完整的登录流程
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @return 登录结果结构体
 */
LoginResult AccountModel::performLogin(const QString &cardNumber, const QString &pin)
{
    LoginResult result;
    result.success = false;

    // 验证登录
    OperationResult validationResult = validateLogin(cardNumber, pin);
    if (!validationResult.success) {
        result.errorMessage = validationResult.errorMessage;
        return result;
    }

    // 获取账户信息
    const Account* account = findAccount(cardNumber);
    if (!account) {
        result.errorMessage = "账户不存在"; // 理论上验证已通过，不应发生
        return result;
    }

    // 设置返回信息
    result.success = true;
    result.isAdmin = account->isAdmin;
    result.holderName = account->holderName;
    result.balance = account->balance;
    result.withdrawLimit = account->withdrawLimit;

    qDebug() << "登录成功，卡号:" << cardNumber << "，用户类型:" << (account->isAdmin ? "管理员" : "普通用户");
    return result;
}

/**
 * @brief 执行完整的管理员登录流程
 * @param cardNumber 管理员卡号
 * @param pin 管理员 PIN 码
 * @return 登录结果结构体
 */
LoginResult AccountModel::performAdminLogin(const QString &cardNumber, const QString &pin)
{
    LoginResult result;
    result.success = false;

    // 验证管理员登录
    OperationResult validationResult = validateAdminLogin(cardNumber, pin);
    if (!validationResult.success) {
        result.errorMessage = validationResult.errorMessage;
        return result;
    }

    // 获取账户信息
    const Account* account = findAccount(cardNumber);
    if (!account) {
        result.errorMessage = "管理员账户不存在"; // 理论上验证已通过，不应发生
        return result;
    }

    // 设置返回信息
    result.success = true;
    result.isAdmin = true;  // 确保是管理员
    result.holderName = account->holderName;
    result.balance = account->balance;
    result.withdrawLimit = account->withdrawLimit;

    qDebug() << "管理员登录成功，卡号:" << cardNumber;
    return result;
}

/**
 * @brief 执行 PIN 码修改操作
 * @param cardNumber 卡号
 * @param currentPin 当前 PIN 码
 * @param newPin 新 PIN 码
 * @param confirmPin 确认新 PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::performPinChange(const QString &cardNumber, const QString &currentPin, const QString &newPin, const QString &confirmPin)
{
    // 首先验证密码修改参数
    OperationResult validationResult = validatePinChange(cardNumber, currentPin, newPin, confirmPin);
    if (!validationResult.success) {
        return validationResult;
    }

    // 执行密码修改
    OperationResult result = changePin(cardNumber, currentPin, newPin);
    if (result.success) {
        return OperationResult::Success();
    } else {
        // 理论上验证已通过，不应发生此情况，但为鲁棒性保留
        return OperationResult::Failure("PIN码修改失败");
    }
}

/**
 * @brief 获取目标账户信息（转账时用于显示）
 * @param cardNumber 当前用户卡号
 * @param targetCardNumber 目标卡号
 * @return 目标账户持卡人姓名，如果账户不存在或验证失败返回空字符串
 */
QString AccountModel::getTargetAccountInfo(const QString &cardNumber, const QString &targetCardNumber)
{
    const Account* account = findAccount(cardNumber);
    const Account* targetAccount = findAccount(targetCardNumber);

    if (!account || !targetAccount) {
        return QString(); // 如果任一账户未找到，返回空字符串
    }

    return targetAccount->holderName;
}

/**
 * @brief 将 Account 对象转换为 QVariantMap
 * @param account 要转换的 Account 对象
 * @return 对应的 QVariantMap
 */
QVariantMap AccountModel::accountToVariantMap(const Account &account) const
{
    QVariantMap accountMap;
    accountMap["cardNumber"] = account.cardNumber;
    accountMap["holderName"] = account.holderName;
    accountMap["balance"] = account.balance;
    accountMap["withdrawLimit"] = account.withdrawLimit;
    accountMap["isLocked"] = account.isLocked;
    accountMap["isAdmin"] = account.isAdmin;
    return accountMap;
}

/**
 * @brief 获取所有账户并转换为 QVariantList
 * @return 包含所有账户数据的 QVariantList
 */
QVariantList AccountModel::getAllAccountsAsVariantList() const
{
    QVariantList result;
    QVector<Account> accounts = getAllAccounts();

    for (const Account &account : accounts) {
        result.append(accountToVariantMap(account));
    }

    return result;
}

/**
 * @brief 重置账户 PIN 码 (管理员功能)
 * @param cardNumber 卡号
 * @param newPin 新 PIN 码
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::resetPin(const QString &cardNumber, const QString &newPin)
{
    // 验证卡号是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("卡号不存在");
    }

    // 验证新 PIN 码格式
    if (newPin.length() != 4 || !newPin.toInt()) {
        return OperationResult::Failure("PIN码必须是4位数字");
    }

    // 获取账户并更新 PIN
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("找不到账户"); // 理论上验证已通过，不应发生
    }

    // 更新 PIN
    account->pin = newPin;

    // 保存更改
    if (saveAccounts()) {
        return OperationResult::Success();
    } else {
        return OperationResult::Failure("保存账户数据失败");
    }
}

/**
 * @brief 从 ViewModel 更新账户信息
 *
 * 更新账户的可编辑字段（持卡人姓名、余额、限额、锁定状态），保留 PIN 和管理员状态。
 *
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param balance 余额
 * @param withdrawLimit 取款限额
 * @param isLocked 是否锁定
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::updateAccountFromViewModel(const QString &cardNumber, const QString &holderName,
                                                     double balance, double withdrawLimit, bool isLocked)
{
    // 验证账户更新参数
    OperationResult result = validateUpdateAccount(cardNumber, holderName, balance, withdrawLimit);
    if (!result.success) {
        return result;
    }

    // 获取现有账户
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("找不到账户"); // 理论上验证已通过，不应发生
    }

    // 更新可以修改的字段，但保留 PIN 和管理员状态
    account->holderName = holderName;
    account->balance = balance;
    account->withdrawLimit = withdrawLimit;
    account->isLocked = isLocked;

    // 保存更改
    if (saveAccounts()) {
        return OperationResult::Success();
    } else {
        return OperationResult::Failure("保存账户数据失败");
    }
}

/**
 * @brief 计算预测余额（包含验证逻辑）
 *
 * 验证输入参数并调用预测余额方法。
 *
 * @param cardNumber 卡号
 * @param transactionModel 交易数据模型指针
 * @param daysInFuture 预测未来天数
 * @param outBalance 输出参数，存储计算出的预测余额
 * @return 操作结果 (成功或失败及错误信息)
 */
OperationResult AccountModel::calculatePredictedBalance(const QString &cardNumber,
                                                      const TransactionModel* transactionModel,
                                                      int daysInFuture,
                                                      double &outBalance)
{
    // 添加更详细的日志
    qDebug() << "计算预测余额开始 - 卡号:" << cardNumber
             << ", 天数:" << daysInFuture
             << ", TransactionModel:" << (transactionModel ? "有效" : "无效")
             << ", 内部存储的TransactionModel:" << (m_transactionModel ? "有效" : "无效");

    // 验证卡号是否存在
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("没有有效的卡号");
    }

    // 验证账户是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }

    // 尝试使用传入的交易模型或内部存储的交易模型
    const TransactionModel* modelToUse = transactionModel;
    if (!modelToUse) {
        modelToUse = m_transactionModel;
        qDebug() << "传入的TransactionModel无效，尝试使用内部存储的TransactionModel";
    }

    // 最终验证是否有可用的交易模型
    if (!modelToUse) {
        qWarning() << "无法计算预测余额：没有可用的TransactionModel";
        return OperationResult::Failure("交易模型无效");
    }

    // 限制预测天数在合理范围内
    if (daysInFuture <= 0) {
        return OperationResult::Failure("预测天数必须大于0");
    }

    if (daysInFuture > 365) {
        return OperationResult::Failure("预测天数过长，请选择1-365天之间");
    }

    // 调用现有方法计算预测余额
    outBalance = predictBalance(cardNumber, modelToUse, daysInFuture);
    qDebug() << "预测余额计算成功 - 结果:" << outBalance;
    return OperationResult::Success();
}