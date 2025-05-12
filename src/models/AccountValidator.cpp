// AccountValidator.cpp
/**
 * @file AccountValidator.cpp
 * @brief 账户验证器类实现
 *
 * 实现了AccountValidator类中定义的所有验证方法。
 */
#include "AccountValidator.h"
#include <QDebug>

/**
 * @brief 构造函数
 * @param repository 账户存储库接口指针
 */
AccountValidator::AccountValidator(IAccountRepository* repository)
    : m_repository(repository)
{
}

/**
 * @brief 验证账户凭据
 * @param cardNumber 卡号
 * @param pin PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validateCredentials(const QString& cardNumber, const QString& pin) const
{
    // 验证输入参数
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请输入卡号");
    }

    if (pin.isEmpty()) {
        return OperationResult::Failure("请输入PIN码");
    }

    // 验证卡号格式
    OperationResult cardNumberResult = validateCardNumberFormat(cardNumber);
    if (!cardNumberResult.success) {
        return cardNumberResult;
    }

    // 查找账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        qDebug() << "验证失败: 卡号不存在:" << cardNumber;
        return OperationResult::Failure("卡号或PIN码错误");
    }

    // 获取可修改的账户副本
    Account account = accountOpt.value();
    
    // 检查账户是否被永久锁定
    if (account.isLocked) {
        qDebug() << "验证失败: 账户已永久锁定:" << cardNumber;
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }
    
    // 检查账户是否被临时锁定
    if (account.isTemporarilyLocked()) {
        qDebug() << "验证失败: 账户已临时锁定:" << cardNumber 
                 << "锁定至:" << account.temporaryLockTime.toString();
        
        return OperationResult::Failure(
            QString("由于多次登录失败，账户已临时锁定，请%1分钟后再试")
                .arg(Account::TEMP_LOCK_DURATION));
    }

    // 验证PIN码（使用哈希比较）
    bool pinMatches = account.verifyPin(pin);
    qDebug() << "PIN验证结果:" << pinMatches << "卡号:" << cardNumber;
    
    if (!pinMatches) {
        // 记录登录失败
        bool lockedNow = account.recordFailedLogin();
        
        // 保存更新后的账户状态
        m_repository->saveAccount(account);
        
        if (lockedNow) {
            return OperationResult::Failure(
                QString("PIN码错误，由于多次登录失败，账户已临时锁定，请%1分钟后再试")
                    .arg(Account::TEMP_LOCK_DURATION));
        }
        
        return OperationResult::Failure(
            QString("卡号或PIN码错误，剩余尝试次数: %1")
                .arg(Account::MAX_FAILED_ATTEMPTS - account.failedLoginAttempts));
    }
    
    // 登录成功，重置失败计数
    if (account.failedLoginAttempts > 0) {
        account.resetFailedLoginAttempts();
        m_repository->saveAccount(account);
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证管理员登录
 * @param cardNumber 卡号
 * @param pin PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validateAdminLogin(const QString& cardNumber, const QString& pin) const
{
    // 首先验证基本凭据
    OperationResult credResult = validateCredentials(cardNumber, pin);
    if (!credResult.success) {
        return credResult;
    }
    
    // 查找账户并确认是管理员
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt.value().isAdmin) {
        qDebug() << "管理员验证失败: 账户不是管理员:" << cardNumber;
        return OperationResult::Failure("此账户没有管理权限");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证账户是否存在
 * @param cardNumber 卡号
 * @return 操作结果，如果账户不存在则返回失败
 */
OperationResult AccountValidator::validateAccountExists(const QString& cardNumber) const
{
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    if (!m_repository->accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证账户是否锁定
 * @param cardNumber 卡号
 * @return 操作结果，如果账户锁定则返回失败
 */
OperationResult AccountValidator::validateAccountNotLocked(const QString& cardNumber) const
{
    // 首先验证账户是否存在
    OperationResult existResult = validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取账户信息
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value(); // 前面已验证账户存在
    
    // 检查账户是否被永久锁定
    if (account.isLocked) {
        return OperationResult::Failure("账户已锁定");
    }
    
    // 检查账户是否被临时锁定
    if (account.isTemporarilyLocked()) {
        return OperationResult::Failure(
            QString("由于多次登录失败，账户已临时锁定，请%1分钟后再试")
                .arg(Account::TEMP_LOCK_DURATION));
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证账户余额是否足够
 * @param cardNumber 卡号
 * @param amount 需要的金额
 * @return 操作结果，如果余额不足则返回失败
 */
OperationResult AccountValidator::validateSufficientBalance(const QString& cardNumber, double amount) const
{
    // 首先验证账户是否存在
    OperationResult existResult = validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取账户信息
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value(); // 前面已验证账户存在
    
    // 检查账户余额是否足够
    if (amount > account.balance) {
        return OperationResult::Failure("余额不足");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证取款限额
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果，如果超过限额则返回失败
 */
OperationResult AccountValidator::validateWithdrawLimit(const QString& cardNumber, double amount) const
{
    // 首先验证账户是否存在
    OperationResult existResult = validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 获取账户信息
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value(); // 前面已验证账户存在
    
    // 检查取款金额是否超过取款限额
    if (amount > account.withdrawLimit) {
        return OperationResult::Failure(QString("超出单次取款限额 %1").arg(account.withdrawLimit));
    }
    
    return OperationResult::Success();
}

/**
 * @brief 通用验证方法，按顺序执行多个验证步骤
 * @param validations 验证函数列表
 * @return 第一个失败的验证结果，或者全部成功时返回成功结果
 */
OperationResult AccountValidator::validateOperation(const std::vector<ValidationFunction>& validations) const
{
    // 按顺序执行每个验证步骤
    for (const auto& validation : validations) {
        OperationResult result = validation();
        if (!result.success) {
            // 如果有任何一个验证步骤失败，立即返回失败结果
            return result;
        }
    }
    
    // 所有验证步骤都通过，返回成功结果
    return OperationResult::Success();
}

/**
 * @brief 验证取款操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateWithdrawal(const QString& cardNumber, double amount) const
{
    // 检查基本输入条件
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }

    if (amount <= 0) {
        return OperationResult::Failure("取款金额必须为正数");
    }

    // 使用通用验证方法，构建验证步骤序列
    return validateOperation({
        // 验证账户是否存在
        [this, cardNumber]() {
            return validateAccountExists(cardNumber);
        },
        // 验证账户是否锁定
        [this, cardNumber]() {
            return validateAccountNotLocked(cardNumber);
        },
        // 验证取款限额
        [this, cardNumber, amount]() {
            return validateWithdrawLimit(cardNumber, amount);
        },
        // 验证余额是否足够
        [this, cardNumber, amount]() {
            return validateSufficientBalance(cardNumber, amount);
        }
    });
}

/**
 * @brief 验证存款操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateDeposit(const QString& cardNumber, double amount) const
{
    // 检查基本输入条件
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }

    if (amount <= 0) {
        return OperationResult::Failure("存款金额必须为正数");
    }

    // 使用通用验证方法，构建验证步骤序列
    return validateOperation({
        // 验证账户是否存在
        [this, cardNumber]() {
            return validateAccountExists(cardNumber);
        },
        // 验证账户是否锁定
        [this, cardNumber]() {
            return validateAccountNotLocked(cardNumber);
        },
        // 检查存款金额是否超过上限 - 增加上限到100万
        [amount]() {
            const double maxDeposit = 1000000.0; // 单次存款上限提高到100万
            if (amount > maxDeposit) {
                return OperationResult::Failure(QString("单次存款不能超过 %1").arg(maxDeposit));
            }
            return OperationResult::Success();
        }
    });
}

/**
 * @brief 验证转账操作
 * @param fromCardNumber 源卡号
 * @param toCardNumber 目标卡号
 * @param amount 转账金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateTransfer(const QString& fromCardNumber, 
                                                  const QString& toCardNumber, 
                                                  double amount) const
{
    // 检查基本输入条件
    if (fromCardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }

    if (toCardNumber.isEmpty()) {
        return OperationResult::Failure("请输入目标卡号");
    }

    if (amount <= 0) {
        return OperationResult::Failure("转账金额必须为正数");
    }

    // 使用通用验证方法，构建验证步骤序列
    return validateOperation({
        // 验证源卡号和目标卡号不能相同
        [fromCardNumber, toCardNumber]() {
            if (fromCardNumber == toCardNumber) {
                return OperationResult::Failure("源卡号和目标卡号不能相同");
            }
            return OperationResult::Success();
        },
        // 验证源账户是否存在
        [this, fromCardNumber]() {
            return validateAccountExists(fromCardNumber);
        },
        // 验证源账户是否锁定
        [this, fromCardNumber]() {
            return validateAccountNotLocked(fromCardNumber);
        },
        // 验证目标账户是否存在
        [this, toCardNumber]() {
            return validateAccountExists(toCardNumber);
        },
        // 验证目标账户是否锁定
        [this, toCardNumber]() {
            return validateAccountNotLocked(toCardNumber);
        },
        // 验证源账户余额是否足够
        [this, fromCardNumber, amount]() {
            return validateSufficientBalance(fromCardNumber, amount);
        },
        // 检查转账金额是否超过单次限额 - 增加上限到100万
        [amount]() {
            const double maxTransfer = 1000000.0; // 单次转账上限提高到100万
            if (amount > maxTransfer) {
                return OperationResult::Failure(QString("单次转账不能超过 %1").arg(maxTransfer));
            }
            return OperationResult::Success();
        }
    });
}

/**
 * @brief 验证PIN码修改
 * @param cardNumber 卡号
 * @param currentPin 当前PIN码
 * @param newPin 新PIN码
 * @param confirmPin 确认新PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validatePinChange(const QString& cardNumber, 
                                      const QString& currentPin,
                                      const QString& newPin, 
                                      const QString& confirmPin) const
{
    // 使用通用验证方法，构建验证步骤序列
    return validateOperation({
        // 验证卡号和当前PIN
        [this, cardNumber, currentPin]() {
            return validateCredentials(cardNumber, currentPin);
        },
        // 验证新PIN格式
        [this, cardNumber, newPin]() {
            // 先获取账户以检查PIN格式
            std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
            if (!accountOpt) {
                return OperationResult::Failure("账户不存在");
            }
            const Account& account = accountOpt.value();
            
            if (!account.isValidPin(newPin)) {
                return OperationResult::Failure("新PIN码格式无效，必须为4-6位数字");
            }
            return OperationResult::Success();
        },
        // 验证确认PIN是否匹配
        [newPin, confirmPin]() {
            // 如果提供了确认PIN，则验证是否匹配
            if (!confirmPin.isEmpty() && newPin != confirmPin) {
                return OperationResult::Failure("两次输入的新PIN码不匹配");
            }
            return OperationResult::Success();
        },
        // 验证新旧PIN是否相同
        [this, cardNumber, newPin]() {
            std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
            if (!accountOpt) {
                return OperationResult::Failure("账户不存在");
            }
            const Account& account = accountOpt.value();
            
            // 验证新旧PIN是否相同
            if (account.verifyPin(newPin)) {
                return OperationResult::Failure("新PIN码不能与当前PIN码相同");
            }
            return OperationResult::Success();
        }
    });
}

/**
 * @brief 验证管理员操作
 * @param cardNumber 管理员卡号
 * @return 操作结果
 */
OperationResult AccountValidator::validateAdminOperation(const QString& cardNumber) const
{
    // 验证卡号
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("管理员卡号不能为空");
    }
    
    // 查找账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("管理员账户不存在");
    }
    
    const Account& account = accountOpt.value();
    
    // 检查是否为管理员账户
    if (!account.isAdmin) {
        return OperationResult::Failure("此账户没有管理权限");
    }
    
    // 检查账户是否被锁定
    if (account.isLocked) {
        return OperationResult::Failure("管理员账户已锁定");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证创建账户操作
 * @param cardNumber 新卡号
 * @param pin PIN码
 * @param holderName 持卡人姓名
 * @param balance 初始余额
 * @param withdrawLimit 取款限额
 * @param isAdmin 是否为管理员
 * @return 操作结果
 */
OperationResult AccountValidator::validateCreateAccount(const QString& cardNumber, 
                                                       const QString& pin, 
                                                       const QString& holderName,
                                                       double balance, 
                                                       double withdrawLimit, 
                                                       bool isAdmin) const
{
    // 使用通用验证方法，构建验证步骤序列
    return validateOperation({
        // 验证卡号格式
        [this, cardNumber]() {
            return validateCardNumberFormat(cardNumber);
        },
        // 验证卡号是否已存在
        [this, cardNumber]() {
            if (m_repository->accountExists(cardNumber)) {
                return OperationResult::Failure("该卡号已存在");
            }
            return OperationResult::Success();
        },
        // 验证PIN码格式
        [this, pin]() {
            return validatePinFormat(pin);
        },
        // 验证持卡人姓名
        [holderName]() {
            if (holderName.isEmpty()) {
                return OperationResult::Failure("持卡人姓名不能为空");
            }
            return OperationResult::Success();
        },
        // 验证余额和取款限额
        [balance, withdrawLimit]() {
            if (balance < 0) {
                return OperationResult::Failure("初始余额不能为负数");
            }
            if (withdrawLimit <= 0) {
                return OperationResult::Failure("取款限额必须为正数");
            }
            return OperationResult::Success();
        }
    });
}

/**
 * @brief 验证更新账户操作
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param balance 余额
 * @param withdrawLimit 取款限额
 * @return 操作结果
 */
OperationResult AccountValidator::validateUpdateAccount(const QString& cardNumber, 
                                                       const QString& holderName,
                                                       double balance, 
                                                       double withdrawLimit) const
{
    // 验证卡号
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    // 验证账户是否存在
    if (!m_repository->accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 验证持卡人姓名
    if (holderName.isEmpty()) {
        return OperationResult::Failure("持卡人姓名不能为空");
    }
    
    // 验证余额和取款限额
    if (balance < 0) {
        return OperationResult::Failure("余额不能为负数");
    }
    
    if (withdrawLimit <= 0) {
        return OperationResult::Failure("取款限额必须为正数");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证PIN码格式
 * @param pin PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validatePinFormat(const QString& pin) const
{
    // 直接使用Account类的静态方法验证PIN格式
    if (!Account::isValidPin(pin)) {
        return OperationResult::Failure("PIN码格式无效，必须为4-6位数字");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证卡号格式
 * @param cardNumber 卡号
 * @return 操作结果
 */
OperationResult AccountValidator::validateCardNumberFormat(const QString& cardNumber) const
{
    // 直接使用Account类的静态方法验证卡号格式
    if (!Account::isValidCardNumber(cardNumber)) {
        return OperationResult::Failure("卡号格式无效，必须为16位数字");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证目标账户
 * @param targetCardNumber 目标卡号
 * @return 操作结果
 */
OperationResult AccountValidator::validateTargetAccount(const QString& targetCardNumber) const
{
    // 验证目标卡号
    if (targetCardNumber.isEmpty()) {
        return OperationResult::Failure("目标卡号不能为空");
    }
    
    // 验证卡号格式
    OperationResult cardNumberResult = validateCardNumberFormat(targetCardNumber);
    if (!cardNumberResult.success) {
        return cardNumberResult;
    }
    
    // 查找目标账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(targetCardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("目标账户不存在");
    }
    
    const Account& account = accountOpt.value();
    
    // 检查账户是否被锁定
    if (account.isLocked) {
        return OperationResult::Failure("目标账户已锁定");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证登录输入参数
 * @param cardNumber 卡号
 * @param pin PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validateLoginInput(const QString& cardNumber, const QString& pin) const
{
    // 基本输入验证
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请输入卡号");
    }

    if (pin.isEmpty()) {
        return OperationResult::Failure("请输入PIN码");
    }
    
    // 验证卡号格式
    return validateCardNumberFormat(cardNumber);
}

/**
 * @brief 验证取款输入参数
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateWithdrawalInput(const QString& cardNumber, double amount) const
{
    // 登录状态验证
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }
    
    // 金额基本验证
    if (amount <= 0) {
        return OperationResult::Failure("取款金额必须为正数");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证存款输入参数
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateDepositInput(const QString& cardNumber, double amount) const
{
    // 登录状态验证
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }
    
    // 金额基本验证
    if (amount <= 0) {
        return OperationResult::Failure("存款金额必须为正数");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证转账输入参数
 * @param cardNumber 卡号
 * @param targetCard 目标卡号
 * @param amount 转账金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateTransferInput(const QString& cardNumber, const QString& targetCard, double amount) const
{
    // 登录状态验证
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }
    
    // a目标卡号验证
    if (targetCard.isEmpty()) {
        return OperationResult::Failure("请输入目标卡号");
    }
    
    // 金额基本验证
    if (amount <= 0) {
        return OperationResult::Failure("转账金额必须为正数");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证PIN码修改输入参数
 * @param cardNumber 卡号
 * @param currentPin 当前PIN码
 * @param newPin 新PIN码
 * @param confirmPin 确认新PIN码
 * @return 操作结果
 */
OperationResult AccountValidator::validatePinChangeInput(const QString& cardNumber, const QString& currentPin, 
                                                 const QString& newPin, const QString& confirmPin) const
{
    // 登录状态验证
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }
    
    // PIN码输入验证
    if (currentPin.isEmpty()) {
        return OperationResult::Failure("请输入当前PIN码");
    }
    
    if (newPin.isEmpty()) {
        return OperationResult::Failure("请输入新PIN码");
    }
    
    if (confirmPin.isEmpty()) {
        return OperationResult::Failure("请确认新PIN码");
    }
    
    // 两次输入PIN一致性验证
    if (newPin != confirmPin) {
        return OperationResult::Failure("两次输入的新PIN码不匹配");
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证是否已登录
 * @param isLoggedIn 是否登录标志
 * @param cardNumber 卡号
 * @return 操作结果
 */
OperationResult AccountValidator::validateLoggedInStatus(bool isLoggedIn, const QString& cardNumber) const
{
    if (!isLoggedIn || cardNumber.isEmpty()) {
        return OperationResult::Failure("请先登录");
    }
    
    return OperationResult::Success();
} 