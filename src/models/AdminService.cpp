// AdminService.cpp
/**
 * @file AdminService.cpp
 * @brief 管理员服务类实现
 *
 * 实现了AdminService类中定义的管理员操作方法。
 */
#include "AdminService.h"
#include <QDebug>

/**
 * @brief 构造函数
 * @param repository 账户存储库
 * @param validator 账户验证器
 * @param transactionModel 交易记录模型
 */
AdminService::AdminService(IAccountRepository* repository, 
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
void AdminService::setTransactionModel(TransactionModel* transactionModel)
{
    m_transactionModel = transactionModel;
}

/**
 * @brief 执行管理员登录
 * @param cardNumber 卡号
 * @param pin PIN码
 * @return 登录结果，包含成功状态和账户信息
 */
LoginResult AdminService::performAdminLogin(const QString& cardNumber, const QString& pin)
{
    // 验证管理员账户凭据
    OperationResult validationResult = m_validator->validateAdminLogin(cardNumber, pin);
    if (!validationResult.success) {
        return LoginResult::Failure(validationResult.errorMessage);
    }
    
    // 获取账户信息 - 验证通过后账户一定存在
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value();
    
    // 返回成功的登录结果，包含账户信息
    return LoginResult::Success(
        true,  // 管理员登录成功，isAdmin 必定为 true
        account.holderName,
        account.balance,
        account.withdrawLimit
    );
}

/**
 * @brief 创建新账户
 * @param cardNumber 卡号
 * @param pin PIN码
 * @param holderName 持卡人姓名
 * @param balance 初始余额
 * @param withdrawLimit 取款限额
 * @param isAdmin 是否为管理员账户
 * @return 操作结果
 */
OperationResult AdminService::createAccount(const QString& cardNumber, 
                                         const QString& pin, 
                                         const QString& holderName,
                                         double balance, 
                                         double withdrawLimit, 
                                         bool isAdmin)
{
    // 验证创建账户操作 - 使用单一验证方法
    OperationResult validationResult = m_validator->validateCreateAccount(
        cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 创建新账户
    Account newAccount(
        cardNumber,
        pin,  // 构造函数中会自动哈希PIN码
        holderName,
        balance,
        withdrawLimit,
        false,  // 新账户默认不锁定
        isAdmin
    );
    
    // 保存账户
    OperationResult saveResult = m_repository->saveAccount(newAccount);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录创建账户操作
    logAdminOperation("", "创建账户", cardNumber, 
                      QString("创建账户: %1, 持卡人: %2").arg(cardNumber).arg(holderName));
    
    return OperationResult::Success();
}

/**
 * @brief 更新现有账户信息
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param balance 账户余额
 * @param withdrawLimit 取款限额
 * @param isLocked 是否锁定账户
 * @return 操作结果
 */
OperationResult AdminService::updateAccount(const QString& cardNumber, 
                                          const QString& holderName,
                                          double balance, 
                                          double withdrawLimit,
                                          bool isLocked)
{
    // 验证更新账户操作 - 使用单一验证方法
    OperationResult validationResult = m_validator->validateUpdateAccount(
        cardNumber, holderName, balance, withdrawLimit);
    if (!validationResult.success) {
        return validationResult;
    }
    
    // 获取现有账户 - 验证通过后账户一定存在
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value();
    
    // 更新账户信息
    account.holderName = holderName;
    account.balance = balance;
    account.withdrawLimit = withdrawLimit;
    account.isLocked = isLocked;
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录更新账户操作
    logAdminOperation("", "更新账户", cardNumber, 
                      QString("更新账户: %1, 持卡人: %2, 余额: %3").arg(cardNumber).arg(holderName).arg(balance));
    
    return OperationResult::Success();
}

/**
 * @brief 删除账户
 * @param cardNumber 要删除的账户卡号
 * @return 操作结果
 */
OperationResult AdminService::deleteAccount(const QString& cardNumber)
{
    // 验证账户是否存在
    OperationResult existResult = m_validator->validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取账户信息（用于日志）
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value();
    
    // 检查是否为管理员账户
    if (account.isAdmin) {
        return OperationResult::Failure("不能删除管理员账户");
    }
    
    // 删除账户
    OperationResult deleteResult = m_repository->deleteAccount(cardNumber);
    if (!deleteResult.success) {
        return deleteResult;
    }
    
    // 如果有交易记录模型，清除相关交易记录
    if (m_transactionModel) {
        m_transactionModel->clearTransactionsForCard(cardNumber);
    }
    
    // 记录删除账户操作
    logAdminOperation("", "删除账户", cardNumber, 
                      QString("删除账户: %1, 持卡人: %2").arg(cardNumber).arg(account.holderName));
    
    return OperationResult::Success();
}

/**
 * @brief 设置账户锁定状态
 * @param cardNumber 卡号
 * @param locked 是否锁定
 * @return 操作结果
 */
OperationResult AdminService::setAccountLockStatus(const QString& cardNumber, bool locked)
{
    // 验证账户是否存在
    OperationResult existResult = m_validator->validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取现有账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value();
    
    // 检查是否为管理员账户
    if (account.isAdmin && locked) {
        return OperationResult::Failure("不能锁定管理员账户");
    }
    
    // 更新锁定状态
    account.isLocked = locked;
    
    // 如果解锁账户，同时清除临时锁定和登录失败计数
    if (!locked) {
        account.resetFailedLoginAttempts();
    }
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录锁定/解锁操作
    QString operationType = locked ? "锁定账户" : "解锁账户";
    logAdminOperation("", operationType, cardNumber, 
                      QString("%1: %2, 持卡人: %3").arg(operationType).arg(cardNumber).arg(account.holderName));
    
    return OperationResult::Success();
}

/**
 * @brief 重置PIN码
 * @param cardNumber 卡号
 * @param newPin 新PIN码
 * @return 操作结果
 */
OperationResult AdminService::resetPin(const QString& cardNumber, const QString& newPin)
{
    // 验证PIN码格式
    OperationResult pinValidationResult = m_validator->validatePinFormat(newPin);
    if (!pinValidationResult.success) {
        return pinValidationResult;
    }
    
    // 验证账户是否存在
    OperationResult existResult = m_validator->validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取现有账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value();
    
    // 使用安全的方法更新PIN码
    account.setPin(newPin);
    
    // 重置账户的失败登录次数和临时锁定
    account.resetFailedLoginAttempts();
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录重置PIN码操作
    logAdminOperation("", "重置安全信息", cardNumber, 
                      QString("重置账户安全信息: %1, 持卡人: %2").arg(cardNumber).arg(account.holderName));
    
    return OperationResult::Success();
}

/**
 * @brief 设置取款限额
 * @param cardNumber 卡号
 * @param limit 新限额
 * @return 操作结果
 */
OperationResult AdminService::setWithdrawLimit(const QString& cardNumber, double limit)
{
    // 验证限额
    if (limit <= 0) {
        return OperationResult::Failure("取款限额必须为正数");
    }
    
    // 验证账户是否存在
    OperationResult existResult = m_validator->validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取现有账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    Account account = accountOpt.value();
    
    // 更新取款限额
    account.withdrawLimit = limit;
    
    // 保存更新后的账户
    OperationResult saveResult = m_repository->saveAccount(account);
    if (!saveResult.success) {
        return saveResult;
    }
    
    // 记录设置取款限额操作
    logAdminOperation("", "设置取款限额", cardNumber, 
                      QString("设置取款限额: %1, 持卡人: %2, 新限额: %3").arg(cardNumber).arg(account.holderName).arg(limit));
    
    return OperationResult::Success();
}

/**
 * @brief 获取所有账户列表
 * @return 所有账户的列表
 */
QVector<Account> AdminService::getAllAccounts() const
{
    return m_repository->getAllAccounts();
}

/**
 * @brief 记录管理员操作日志
 * @param adminCardNumber 管理员卡号
 * @param operationType 操作类型
 * @param targetCardNumber 目标卡号
 * @param description 操作描述
 */
void AdminService::logAdminOperation(const QString& adminCardNumber, 
                                   const QString& operationType,
                                   const QString& targetCardNumber, 
                                   const QString& description)
{
    // 不记录登录、登出和PIN码相关的操作
    if (operationType.contains("登录") || 
        operationType.contains("登出") || 
        operationType.contains("PIN码")) {
        return;
    }
    
    if (m_transactionModel) {
        // 使用操作类型作为交易描述前缀
        QString fullDescription = operationType;
        if (!description.isEmpty()) {
            fullDescription += ": " + description;
        }
        
        // 如果提供了管理员卡号，记录到管理员账户
        if (!adminCardNumber.isEmpty()) {
            std::optional<Account> adminAccountOpt = m_repository->findByCardNumber(adminCardNumber);
            if (adminAccountOpt) {
                m_transactionModel->recordTransaction(
                    adminCardNumber,
                    TransactionType::Other,
                    0.0,
                    adminAccountOpt.value().balance,
                    fullDescription,
                    targetCardNumber
                );
            }
        }
        
        // 如果是针对特定账户的操作，也记录到目标账户
        if (!targetCardNumber.isEmpty() && targetCardNumber != adminCardNumber) {
            std::optional<Account> targetAccountOpt = m_repository->findByCardNumber(targetCardNumber);
            if (targetAccountOpt) {
                m_transactionModel->recordTransaction(
                    targetCardNumber,
                    TransactionType::Other,
                    0.0,
                    targetAccountOpt.value().balance,
                    "管理员操作: " + fullDescription,
                    adminCardNumber
                );
            }
        }
    }
}

/**
 * @brief 检查管理员权限
 * @param cardNumber 卡号
 * @return 操作结果
 */
OperationResult AdminService::checkAdminPermission(const QString& cardNumber) const
{
    // 验证管理员权限
    return m_validator->validateAdminOperation(cardNumber);
} 