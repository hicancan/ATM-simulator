// AccountModel.cpp
/**
 * @file AccountModel.cpp
 * @brief 账户数据模型门面类实现
 *
 * 实现了各种门面方法，委托调用到专门的服务类。
 */
#include "AccountModel.h"
#include <QDir>
#include <QStandardPaths>
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象指针
 */
AccountModel::AccountModel(QObject *parent)
    : QObject(parent)
    , m_transactionModel(nullptr)
{
    // 创建账户存储库
    m_repository = std::make_unique<JsonAccountRepository>();
    
    // 创建验证器
    m_validator = std::make_unique<AccountValidator>(m_repository.get());
    
    // 创建各种服务
    m_accountService = std::make_unique<AccountService>(m_repository.get(), m_validator.get());
    m_adminService = std::make_unique<AdminService>(m_repository.get(), m_validator.get());
    
    qDebug() << "AccountModel 门面类初始化完成";
}

/**
 * @brief 析构函数
 */
AccountModel::~AccountModel() = default;

/**
 * @brief 设置交易模型
 * @param transactionModel 交易模型指针
 */
void AccountModel::setTransactionModel(TransactionModel* transactionModel)
{
    m_transactionModel = transactionModel;
    
    if (m_accountService) {
        m_accountService->setTransactionModel(transactionModel);
    }
    
    if (m_adminService) {
        m_adminService->setTransactionModel(transactionModel);
    }
    
    // 创建分析服务（需要交易模型）
    if (transactionModel) {
        m_analyticsService = std::make_unique<AccountAnalyticsService>(
            m_repository.get(), 
            transactionModel
        );
    }
    
    qDebug() << "设置交易模型完成";
}

// =============================
// === AccountService 委托方法 ===
// =============================

LoginResult AccountModel::performLogin(const QString &cardNumber, const QString &pin)
{
    return m_accountService->performLogin(cardNumber, pin);
}

OperationResult AccountModel::withdrawAmount(const QString &cardNumber, double amount)
{
    return m_accountService->withdrawAmount(cardNumber, amount);
}

OperationResult AccountModel::depositAmount(const QString &cardNumber, double amount)
{
    return m_accountService->depositAmount(cardNumber, amount);
}

OperationResult AccountModel::transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount)
{
    return m_accountService->transferAmount(fromCardNumber, toCardNumber, amount);
}

OperationResult AccountModel::changePin(const QString &cardNumber, const QString &currentPin, 
                                       const QString &newPin, const QString &confirmPin)
{
    return m_accountService->changePin(cardNumber, currentPin, newPin, confirmPin);
}

double AccountModel::getBalance(const QString &cardNumber) const
{
    return m_accountService->getBalance(cardNumber);
}

QString AccountModel::getHolderName(const QString &cardNumber) const
{
    return m_accountService->getHolderName(cardNumber);
}

double AccountModel::getWithdrawLimit(const QString &cardNumber) const
{
    return m_accountService->getWithdrawLimit(cardNumber);
}

bool AccountModel::isAccountLocked(const QString &cardNumber) const
{
    return m_accountService->isAccountLocked(cardNumber);
}

// =============================
// === AdminService 委托方法 ===
// =============================

LoginResult AccountModel::performAdminLogin(const QString &cardNumber, const QString &pin)
{
    return m_adminService->performAdminLogin(cardNumber, pin);
}

OperationResult AccountModel::createAccount(const QString &cardNumber, const QString &pin, 
                                           const QString &holderName, double balance, 
                                           double withdrawLimit, bool isAdmin)
{
    return m_adminService->createAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
}

OperationResult AccountModel::updateAccount(const QString &cardNumber, const QString &holderName,
                                           double balance, double withdrawLimit, bool isLocked)
{
    return m_adminService->updateAccount(cardNumber, holderName, balance, withdrawLimit, isLocked);
}

OperationResult AccountModel::deleteAccount(const QString &cardNumber)
{
    return m_adminService->deleteAccount(cardNumber);
}

OperationResult AccountModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    return m_adminService->setAccountLockStatus(cardNumber, locked);
}

OperationResult AccountModel::resetPin(const QString &cardNumber, const QString &newPin)
{
    return m_adminService->resetPin(cardNumber, newPin);
}

OperationResult AccountModel::setWithdrawLimit(const QString &cardNumber, double limit)
{
    return m_adminService->setWithdrawLimit(cardNumber, limit);
}

QVector<Account> AccountModel::getAllAccounts() const
{
    return m_adminService->getAllAccounts();
}

// ====================================
// === AccountAnalyticsService 委托方法 ===
// ====================================

double AccountModel::predictBalance(const QString &cardNumber, int daysInFuture) const
{
    if (!m_analyticsService) {
        qWarning() << "分析服务不可用，无法预测余额";
        return getBalance(cardNumber);
    }
    return m_analyticsService->predictBalance(cardNumber, daysInFuture);
}

OperationResult AccountModel::calculatePredictedBalance(const QString &cardNumber,
                                                      int daysInFuture,
                                                      double &outBalance) const
{
    if (!m_analyticsService) {
        outBalance = getBalance(cardNumber);
        return OperationResult::Failure("分析服务不可用，返回当前余额");
    }
    return m_analyticsService->calculatePredictedBalance(cardNumber, daysInFuture, outBalance);
}

OperationResult AccountModel::predictBalanceMultiDays(const QString &cardNumber,
                                                    const QVector<int> &days,
                                                    QMap<int, double> &outPredictions) const
{
    if (!m_analyticsService) {
        double currentBalance = getBalance(cardNumber);
        for (int day : days) {
            outPredictions[day] = currentBalance;
        }
        return OperationResult::Failure("分析服务不可用，返回当前余额");
    }
    return m_analyticsService->predictBalanceMultiDays(cardNumber, days, outPredictions);
}

OperationResult AccountModel::getAccountTrend(const QString &cardNumber,
                                             int days,
                                             QMap<QDate, double> &outIncomeTrend,
                                             QMap<QDate, double> &outExpenseTrend) const
{
    if (!m_analyticsService) {
        return OperationResult::Failure("分析服务不可用");
    }
    return m_analyticsService->getAccountTrend(cardNumber, days, outIncomeTrend, outExpenseTrend);
}

double AccountModel::getTransactionFrequency(const QString &cardNumber, int days) const
{
    if (!m_analyticsService) {
        return 0.0;
    }
    return m_analyticsService->getTransactionFrequency(cardNumber, days);
}

QVariantList AccountModel::getAllAccountsAsVariantList() const
{
    QVariantList result;
    QVector<Account> accounts = getAllAccounts();
    
    for (const Account& account : accounts) {
        QVariantMap accountMap;
        accountMap["cardNumber"] = account.cardNumber;
        accountMap["holderName"] = account.holderName;
        accountMap["balance"] = account.balance;
        accountMap["withdrawLimit"] = account.withdrawLimit;
        accountMap["isLocked"] = account.isLocked;
        accountMap["isAdmin"] = account.isAdmin;
        result.append(accountMap);
    }
    
    return result;
}