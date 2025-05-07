#include "AccountModel.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <numeric> // Required for std::accumulate
#include <algorithm> // Required for std::min

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
        qDebug() << "验证失败: 卡号不存在:" << cardNumber;
        return false;  // Card not found
    }
    
    if (account->isLocked) {
        qDebug() << "验证失败: 账户已锁定:" << cardNumber;
        return false;  // Account is locked
    }
    
    bool pinMatches = (account->pin == pin);
    qDebug() << "PIN验证结果:" << pinMatches << "输入PIN:" << pin << "存储PIN:" << account->pin;
    return pinMatches;
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
    return account ? account->isLocked : false;
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
    account3.isLocked = true;  // This account is locked
    account3.isAdmin = false;
    addAccount(account3);
    
    // 添加管理员账户
    Account adminAccount;
    adminAccount.cardNumber = "9999888877776666";
    // 确保管理员PIN码为数字字符串，而不是数值
    adminAccount.pin = QString("8888");
    adminAccount.holderName = "管理员";
    adminAccount.balance = 50000.0;
    adminAccount.withdrawLimit = 10000.0;
    adminAccount.isLocked = false;
    adminAccount.isAdmin = true; // 设置为管理员
    
    qDebug() << "管理员账户初始化 PIN码:" << adminAccount.pin;
    addAccount(adminAccount);
    
    // 再次检查管理员账户是否正确保存
    const Account* savedAdmin = findAccount("9999888877776666");
    if (savedAdmin) {
        qDebug() << "管理员账户保存后 PIN码:" << savedAdmin->pin;
    } else {
        qDebug() << "警告: 管理员账户保存失败!";
    }
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
            
            // 确保PIN码为字符串格式
            if (account.cardNumber == "9999888877776666") {
                qDebug() << "加载管理员账户，PIN码:" << account.pin;
                // 确保PIN码正确
                if (account.pin != "8888") {
                    qWarning() << "管理员PIN码不正确，重新设置为8888";
                    account.pin = "8888";
                }
            }
            
            m_accounts[account.cardNumber] = account;
        }
    }
    
    // 确保管理员账户加载正确
    const Account* adminAccount = findAccount("9999888877776666");
    if (adminAccount) {
        qDebug() << "管理员账户加载成功，PIN码:" << adminAccount->pin;
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

double AccountModel::predictBalance(const QString &cardNumber, const TransactionModel* transactionModel, int daysInFuture) const
{
    if (!transactionModel) {
        qWarning() << "TransactionModel is null, cannot predict balance.";
        return getBalance(cardNumber); // Return current balance if no transaction model
    }

    QVector<Transaction> transactions = transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.size() < 2) {
        qWarning() << "Not enough transactions to predict balance for card:" << cardNumber;
        return getBalance(cardNumber); // Return current balance if not enough data
    }

    // Sort transactions by date just in case they are not already sorted
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp < b.timestamp;
    });

    // Use the last N transactions for a more relevant trend (e.g., last 10 or all if fewer)
    // Explicitly cast one of the arguments to int to resolve ambiguity for std::min
    int N = std::min(10, static_cast<int>(transactions.size()));
    if (N < 2) return getBalance(cardNumber);

    QVector<double> balanceChanges;
    QVector<long long> timeDiffsSeconds; // Time differences in seconds

    for (int i = transactions.size() - N + 1; i < transactions.size(); ++i) {
        balanceChanges.append(transactions[i].balanceAfter - transactions[i-1].balanceAfter);
        timeDiffsSeconds.append(transactions[i].timestamp.toSecsSinceEpoch() - transactions[i-1].timestamp.toSecsSinceEpoch());
    }

    if (balanceChanges.isEmpty() || timeDiffsSeconds.isEmpty()) {
        return getBalance(cardNumber);
    }

    double totalBalanceChange = std::accumulate(balanceChanges.constBegin(), balanceChanges.constEnd(), 0.0);
    long long totalTimeDiffSeconds = std::accumulate(timeDiffsSeconds.constBegin(), timeDiffsSeconds.constEnd(), 0LL);

    if (totalTimeDiffSeconds == 0) {
        // Avoid division by zero if all transactions happened at the exact same second (unlikely but possible)
        return transactions.last().balanceAfter; 
    }

    // Average change per second
    double averageChangePerSecond = totalBalanceChange / totalTimeDiffSeconds;
    
    // Average daily change
    double averageDailyChange = averageChangePerSecond * (24 * 60 * 60);

    double currentBalance = transactions.last().balanceAfter;
    double predictedBalance = currentBalance + (averageDailyChange * daysInFuture);

    qDebug() << "Predicting balance for card:" << cardNumber;
    qDebug() << "Current Balance:" << currentBalance;
    qDebug() << "Number of transactions used for trend:" << N-1;
    qDebug() << "Total Balance Change:" << totalBalanceChange;
    qDebug() << "Total Time Difference (seconds):" << totalTimeDiffSeconds;
    qDebug() << "Average Daily Change:" << averageDailyChange;
    qDebug() << "Predicted Balance in" << daysInFuture << "days:" << predictedBalance;

    return predictedBalance;
}

bool AccountModel::isAdmin(const QString &cardNumber) const
{
    const Account* account = findAccount(cardNumber);
    return account ? account->isAdmin : false;
}

QVector<Account> AccountModel::getAllAccounts() const
{
    QVector<Account> accounts;
    for (const auto &account : m_accounts) {
        accounts.append(account);
    }
    return accounts;
}

bool AccountModel::createAccount(const Account &account)
{
    // Check if account with this card number already exists
    if (accountExists(account.cardNumber)) {
        qWarning() << "创建账户失败: 卡号" << account.cardNumber << "已存在";
        return false;
    }
    
    // Add the account
    addAccount(account);
    saveAccounts();
    qDebug() << "成功创建新账户，卡号:" << account.cardNumber;
    return true;
}

bool AccountModel::updateAccount(const Account &account)
{
    Account* existingAccount = findAccount(account.cardNumber);
    if (!existingAccount) {
        qWarning() << "更新账户失败: 卡号" << account.cardNumber << "不存在";
        return false;
    }
    
    // Update account details
    *existingAccount = account;
    saveAccounts();
    qDebug() << "成功更新账户信息，卡号:" << account.cardNumber;
    return true;
}

bool AccountModel::deleteAccount(const QString &cardNumber)
{
    if (!accountExists(cardNumber)) {
        qWarning() << "删除账户失败: 卡号" << cardNumber << "不存在";
        return false;
    }
    
    m_accounts.remove(cardNumber);
    saveAccounts();
    qDebug() << "成功删除账户，卡号:" << cardNumber;
    return true;
}

bool AccountModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        qWarning() << "设置账户锁定状态失败: 卡号" << cardNumber << "不存在";
        return false;
    }
    
    account->isLocked = locked;
    saveAccounts();
    qDebug() << "成功" << (locked ? "锁定" : "解锁") << "账户，卡号:" << cardNumber;
    return true;
}

bool AccountModel::setWithdrawLimit(const QString &cardNumber, double limit)
{
    Account* account = findAccount(cardNumber);
    if (!account) {
        qWarning() << "设置取款限额失败: 卡号" << cardNumber << "不存在";
        return false;
    }
    
    if (limit < 0) {
        qWarning() << "设置取款限额失败: 限额不能为负数";
        return false;
    }
    
    account->withdrawLimit = limit;
    saveAccounts();
    qDebug() << "成功设置取款限额为" << limit << "，卡号:" << cardNumber;
    return true;
}

// 在文件末尾添加新的业务逻辑方法实现

// 验证登录逻辑
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

// 验证管理员登录
OperationResult AccountModel::validateAdminLogin(const QString &cardNumber, const QString &pin)
{
    // 特殊处理管理员账户验证
    if (cardNumber != "9999888877776666" || pin != "8888") {
        return OperationResult::Failure("管理员卡号或PIN码错误");
    }
    
    return OperationResult::Success();
}

// 验证取款逻辑
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

// 验证存款逻辑
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

// 验证转账逻辑
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
        return OperationResult::Failure("账户不存在");
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

// 验证密码修改逻辑
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

// 验证创建账户逻辑
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

// 验证更新账户逻辑
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

// 执行取款操作
OperationResult AccountModel::performWithdrawal(const QString &cardNumber, double amount)
{
    // 首先验证
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

// 执行存款操作
OperationResult AccountModel::performDeposit(const QString &cardNumber, double amount)
{
    // 首先验证
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

// 执行转账操作
OperationResult AccountModel::performTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    // 首先验证
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

// 获取目标账户信息（用于转账显示）
QString AccountModel::getTargetAccountInfo(const QString &cardNumber, const QString &targetCardNumber)
{
    const Account* account = findAccount(cardNumber);
    const Account* targetAccount = findAccount(targetCardNumber);
    
    if (!account || !targetAccount) {
        return QString();
    }
    
    return targetAccount->holderName;
}

// 在文件末尾添加新的方法实现

// 将 Account 对象转换为 QVariantMap
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

// 获取所有账户并转换为 QVariantList 格式
QVariantList AccountModel::getAllAccountsAsVariantList() const
{
    QVariantList result;
    QVector<Account> accounts = getAllAccounts();
    
    for (const Account &account : accounts) {
        result.append(accountToVariantMap(account));
    }
    
    return result;
}

// 执行重置PIN的方法
OperationResult AccountModel::resetPin(const QString &cardNumber, const QString &newPin)
{
    // 验证卡号是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("卡号不存在");
    }
    
    // 验证新PIN格式
    if (newPin.length() != 4 || !newPin.toInt()) {
        return OperationResult::Failure("PIN码必须是4位数字");
    }
    
    // 获取账户并更新PIN
    Account* account = findAccount(cardNumber);
    if (!account) {
        return OperationResult::Failure("找不到账户");
    }
    
    // 更新PIN
    account->pin = newPin;
    
    // 保存更改
    if (saveAccounts()) {
        return OperationResult::Success();
    } else {
        return OperationResult::Failure("保存账户数据失败");
    }
}

// 从 ViewModel 更新账户，保留 PIN 和管理员状态
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
        return OperationResult::Failure("找不到账户");
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

// 管理员权限验证方法
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

// 实现完整的预测余额方法，包含验证逻辑
OperationResult AccountModel::calculatePredictedBalance(const QString &cardNumber, 
                                                      const TransactionModel* transactionModel, 
                                                      int daysInFuture, 
                                                      double &outBalance)
{
    // 验证卡号是否存在
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("没有有效的卡号");
    }
    
    // 验证账户是否存在
    if (!accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 验证TransactionModel是否有效
    if (!transactionModel) {
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
    outBalance = predictBalance(cardNumber, transactionModel, daysInFuture);
    return OperationResult::Success();
}