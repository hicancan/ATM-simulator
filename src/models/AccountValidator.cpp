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
 * @brief 验证金额是否为100的倍数
 * @param amount 金额
 * @param operationType 操作类型(如"取款"、"存款")
 * @return 操作结果，如果金额不是100的倍数则返回失败
 */
OperationResult AccountValidator::validateAmountMultipleOf100(double amount, const QString& operationType) const
{
    // 验证金额是否为正数
    if (amount <= 0) {
        return OperationResult::Failure(operationType + "金额必须为正数");
    }
    
    // 检查金额是否为100的倍数
    if (std::fmod(amount, 100) != 0) {
        return OperationResult::Failure(operationType + "金额必须为100的倍数");
    }
    
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
    // 验证金额是否为100的倍数
    OperationResult amountResult = validateAmountMultipleOf100(amount, "取款");
    if (!amountResult.success) {
        return amountResult;
    }
    
    // 验证账户是否存在
    OperationResult existResult = validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 验证账户是否锁定
    OperationResult lockResult = validateAccountNotLocked(cardNumber);
    if (!lockResult.success) {
        return lockResult;
    }
    
    // 验证取款限额
    OperationResult limitResult = validateWithdrawLimit(cardNumber, amount);
    if (!limitResult.success) {
        return limitResult;
    }
    
    // 验证余额是否足够
    OperationResult balanceResult = validateSufficientBalance(cardNumber, amount);
    if (!balanceResult.success) {
        return balanceResult;
    }
    
    return OperationResult::Success();
}

/**
 * @brief 验证存款操作
 * @param cardNumber 卡号
 * @param amount 存款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateDeposit(const QString& cardNumber, double amount) const
{
    // 验证金额是否为100的倍数
    OperationResult amountResult = validateAmountMultipleOf100(amount, "存款");
    if (!amountResult.success) {
        return amountResult;
    }
    
    // 验证账户是否存在
    OperationResult existResult = validateAccountExists(cardNumber);
    if (!existResult.success) {
        return existResult;
    }
    
    // 验证账户是否锁定
    OperationResult lockResult = validateAccountNotLocked(cardNumber);
    if (!lockResult.success) {
        return lockResult;
    }
    
    // 检查存款金额是否超过上限
    const double maxDeposit = 50000.0; // 单次存款上限
    if (amount > maxDeposit) {
        return OperationResult::Failure(QString("单次存款不能超过 %1").arg(maxDeposit));
    }
    
    return OperationResult::Success();
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
    // 验证源卡号和目标卡号不能相同
    if (fromCardNumber == toCardNumber) {
        return OperationResult::Failure("源卡号和目标卡号不能相同");
    }
    
    // 验证金额是否为正数
    if (amount <= 0) {
        return OperationResult::Failure("转账金额必须为正数");
    }
    
    // 验证源账户是否存在
    OperationResult fromExistResult = validateAccountExists(fromCardNumber);
    if (!fromExistResult.success) {
        return fromExistResult;
    }
    
    // 验证源账户是否锁定
    OperationResult fromLockResult = validateAccountNotLocked(fromCardNumber);
    if (!fromLockResult.success) {
        return fromLockResult;
    }
    
    // 验证目标账户是否存在
    OperationResult toExistResult = validateAccountExists(toCardNumber);
    if (!toExistResult.success) {
        return toExistResult;
    }
    
    // 验证目标账户是否锁定
    OperationResult toLockResult = validateAccountNotLocked(toCardNumber);
    if (!toLockResult.success) {
        return toLockResult;
    }
    
    // 验证源账户余额是否足够
    OperationResult balanceResult = validateSufficientBalance(fromCardNumber, amount);
    if (!balanceResult.success) {
        return balanceResult;
    }
    
    // 检查转账金额是否超过单次限额
    const double maxTransfer = 50000.0; // 单次转账上限
    if (amount > maxTransfer) {
        return OperationResult::Failure(QString("单次转账不能超过 %1").arg(maxTransfer));
    }
    
    return OperationResult::Success();
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
    // 验证卡号和当前PIN
    OperationResult credResult = validateCredentials(cardNumber, currentPin);
    if (!credResult.success) {
        return credResult;
    }
    
    // 获取账户信息
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    const Account& account = accountOpt.value();
    
    // 验证新PIN格式
    if (!account.isValidPin(newPin)) {
        return OperationResult::Failure("新PIN码格式无效，必须为4-6位数字");
    }
    
    // 如果提供了确认PIN，则验证是否匹配
    if (!confirmPin.isEmpty() && newPin != confirmPin) {
        return OperationResult::Failure("两次输入的新PIN码不匹配");
    }
    
    // 验证新旧PIN是否相同
    if (account.verifyPin(newPin)) {
        return OperationResult::Failure("新PIN码不能与当前PIN码相同");
    }
    
    return OperationResult::Success();
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
    // 验证卡号格式
    OperationResult cardNumberResult = validateCardNumberFormat(cardNumber);
    if (!cardNumberResult.success) {
        return cardNumberResult;
    }
    
    // 验证卡号是否已存在
    if (m_repository->accountExists(cardNumber)) {
        return OperationResult::Failure("该卡号已存在");
    }
    
    // 验证PIN码格式
    OperationResult pinResult = validatePinFormat(pin);
    if (!pinResult.success) {
        return pinResult;
    }
    
    // 验证持卡人姓名
    if (holderName.isEmpty()) {
        return OperationResult::Failure("持卡人姓名不能为空");
    }
    
    // 验证余额和取款限额
    if (balance < 0) {
        return OperationResult::Failure("初始余额不能为负数");
    }
    
    if (withdrawLimit <= 0) {
        return OperationResult::Failure("取款限额必须为正数");
    }
    
    return OperationResult::Success();
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