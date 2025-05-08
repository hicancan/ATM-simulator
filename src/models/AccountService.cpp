// AccountService.cpp
/**
 * @file AccountService.cpp
 * @brief 账户服务类实现
 *
 * 实现了AccountService类中定义的账户操作方法。
 */
#include "AccountService.h"
#include <QDebug>

/**
 * @brief 构造函数
 * @param repository 账户存储库
 * @param validator 账户验证器
 * @param transactionModel 交易记录模型
 */
AccountService::AccountService(IAccountRepository* repository, 
                             AccountValidator* validator,
                             TransactionModel* transactionModel)
    : m_repository(repository)
    , m_validator(validator)
    , m_transactionModel(transactionModel)
{
}

/**
 * @brief 设置交易记录模型
 * @param transactionModel 交易记录模型
 */
void AccountService::setTransactionModel(TransactionModel* transactionModel)
{
    m_transactionModel = transactionModel;
}

/**
 * @brief 执行用户登录
 * @param cardNumber 卡号
 * @param pin PIN码
 * @return 登录结果，包含成功状态和账户信息
 */
LoginResult AccountService::performLogin(const QString& cardNumber, const QString& pin)
{
    // 验证账户凭据
    OperationResult validationResult = m_validator->validateCredentials(cardNumber, pin);
    if (!validationResult.success) {
        return LoginResult::Failure(validationResult.errorMessage);
    }
    
    // 获取账户信息
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value(); // 验证通过后一定存在
    
    // 记录登录交易
    if (m_transactionModel) {
        recordTransaction(
            cardNumber,
            TransactionType::Other,
            0.0,
            account.balance,
            "用户登录",
            QString()
        );
    }
    
    // 返回成功的登录结果，包含账户信息
    return LoginResult::Success(
        account.isAdmin,
        account.holderName,
        account.balance,
        account.withdrawLimit
    );
}

/**
 * @brief 执行取款操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果
 */
OperationResult AccountService::withdrawAmount(const QString& cardNumber, double amount)
{
    // 验证取款操作
    OperationResult validationResult = m_validator->validateWithdrawal(cardNumber, amount);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 获取账户并执行取款
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value(); // 验证通过后一定存在
    
    // 更新余额
    account.balance -= amount;
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录取款交易
    if (m_transactionModel) {
        recordTransaction(
            cardNumber,
            TransactionType::Withdrawal,
            amount,
            account.balance,
            "取款",
            QString()
        );
    }
    
    return OperationResult::Success();
}

/**
 * @brief 执行存款操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果
 */
OperationResult AccountService::depositAmount(const QString& cardNumber, double amount)
{
    // 验证存款操作
    OperationResult validationResult = m_validator->validateDeposit(cardNumber, amount);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 获取账户并执行存款
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value(); // 验证通过后一定存在
    
    // 更新余额
    account.balance += amount;
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录存款交易
    if (m_transactionModel) {
        recordTransaction(
            cardNumber,
            TransactionType::Deposit,
            amount,
            account.balance,
            "存款",
            QString()
        );
    }
    
    return OperationResult::Success();
}

/**
 * @brief 执行转账操作
 * @param fromCardNumber 源卡号
 * @param toCardNumber 目标卡号
 * @param amount 转账金额
 * @return 操作结果
 */
OperationResult AccountService::transferAmount(const QString& fromCardNumber, 
                                              const QString& toCardNumber, 
                                              double amount)
{
    // 验证转账操作
    OperationResult validationResult = m_validator->validateTransfer(fromCardNumber, toCardNumber, amount);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 获取源账户和目标账户
    std::optional<Account> fromAccountOpt = m_repository->findByCardNumber(fromCardNumber);
    std::optional<Account> toAccountOpt = m_repository->findByCardNumber(toCardNumber);
    
    Account fromAccount = fromAccountOpt.value(); // 验证通过后一定存在
    Account toAccount = toAccountOpt.value(); // 验证通过后一定存在
    
    // 更新余额
    fromAccount.balance -= amount;
    toAccount.balance += amount;
    
    // 保存更新后的账户
    OperationResult saveFromResult = m_repository->saveAccount(fromAccount);
    if (!saveFromResult.success) {
        return saveFromResult;
    }
    
    OperationResult saveToResult = m_repository->saveAccount(toAccount);
    if (!saveToResult.success) {
        // 如果保存目标账户失败，需要回滚源账户的更改
        fromAccount.balance += amount;
        m_repository->saveAccount(fromAccount);
        return saveToResult;
    }
    
    // 记录转账交易
    if (m_transactionModel) {
        // 记录源账户的转出交易
        recordTransaction(
            fromCardNumber,
            TransactionType::Transfer,
            amount,
            fromAccount.balance,
            QString("转账给 %1").arg(toAccount.holderName),
            toCardNumber
        );
        
        // 记录目标账户的转入交易
        recordTransaction(
            toCardNumber,
            TransactionType::Deposit,
            amount,
            toAccount.balance,
            QString("来自 %1 的转账").arg(fromAccount.holderName),
            fromCardNumber
        );
    }
    
    return OperationResult::Success();
}

/**
 * @brief 修改PIN码
 * @param cardNumber 卡号
 * @param currentPin 当前PIN码
 * @param newPin 新PIN码
 * @param confirmPin 确认新PIN码
 * @return 操作结果
 */
OperationResult AccountService::changePin(const QString& cardNumber, 
                                         const QString& currentPin, 
                                         const QString& newPin, 
                                         const QString& confirmPin)
{
    // 验证PIN码修改操作
    OperationResult validationResult = m_validator->validatePinChange(cardNumber, currentPin, newPin, confirmPin);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 获取账户并修改PIN码
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value(); // 验证通过后一定存在
    
    // 更新PIN码
    account.pin = newPin;
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录PIN码修改交易
    if (m_transactionModel) {
        recordTransaction(
            cardNumber,
            TransactionType::Other,
            0.0,
            account.balance,
            "修改PIN码",
            QString()
        );
    }
    
    return OperationResult::Success();
}

/**
 * @brief 获取账户余额
 * @param cardNumber 卡号
 * @return 账户余额
 */
double AccountService::getBalance(const QString& cardNumber) const
{
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    return accountOpt ? accountOpt.value().balance : 0.0;
}

/**
 * @brief 获取持卡人姓名
 * @param cardNumber 卡号
 * @return 持卡人姓名
 */
QString AccountService::getHolderName(const QString& cardNumber) const
{
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    return accountOpt ? accountOpt.value().holderName : QString();
}

/**
 * @brief 获取账户取款限额
 * @param cardNumber 卡号
 * @return 取款限额
 */
double AccountService::getWithdrawLimit(const QString& cardNumber) const
{
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    return accountOpt ? accountOpt.value().withdrawLimit : 0.0;
}

/**
 * @brief 检查账户是否被锁定
 * @param cardNumber 卡号
 * @return 如果账户被锁定返回true，否则返回false
 */
bool AccountService::isAccountLocked(const QString& cardNumber) const
{
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    return accountOpt ? accountOpt.value().isLocked : false;
}

/**
 * @brief 记录交易
 * @param cardNumber 交易涉及的卡号
 * @param type 交易类型
 * @param amount 交易金额
 * @param balanceAfter 交易后余额
 * @param description 交易描述
 * @param targetCard 目标卡号 (转账时使用)
 */
void AccountService::recordTransaction(const QString& cardNumber, 
                                      TransactionType type,
                                      double amount, 
                                      double balanceAfter,
                                      const QString& description, 
                                      const QString& targetCard)
{
    if (m_transactionModel) {
        m_transactionModel->recordTransaction(
            cardNumber,
            type,
            amount,
            balanceAfter,
            description,
            targetCard
        );
    }
}