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

    // 验证账户状态
    const Account& account = accountOpt.value();
    if (account.isLocked) {
        qDebug() << "验证失败: 账户已锁定:" << cardNumber;
        return OperationResult::Failure("该账户已被锁定，请联系管理员");
    }

    // 验证PIN码
    bool pinMatches = (account.pin == pin);
    qDebug() << "PIN验证结果:" << pinMatches << "输入PIN:" << pin << "存储PIN:" << account.pin;
    
    if (!pinMatches) {
        return OperationResult::Failure("卡号或PIN码错误");
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
 * @brief 验证取款操作
 * @param cardNumber 卡号
 * @param amount 取款金额
 * @return 操作结果
 */
OperationResult AccountValidator::validateWithdrawal(const QString& cardNumber, double amount) const
{
    // 验证卡号
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    // 验证金额
    if (amount <= 0) {
        return OperationResult::Failure("取款金额必须为正数");
    }
    
    // 检查金额是否为100的倍数
    if (std::fmod(amount, 100) != 0) {
        return OperationResult::Failure("取款金额必须为100的倍数");
    }
    
    // 查找账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("账户不存在");
    }
    
    const Account& account = accountOpt.value();
    
    // 检查账户是否被锁定
    if (account.isLocked) {
        return OperationResult::Failure("账户已锁定");
    }
    
    // 检查取款金额是否超过取款限额
    if (amount > account.withdrawLimit) {
        return OperationResult::Failure(QString("超出单次取款限额 %1").arg(account.withdrawLimit));
    }
    
    // 检查账户余额是否足够
    if (amount > account.balance) {
        return OperationResult::Failure("余额不足");
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
    // 验证卡号
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    // 验证金额
    if (amount <= 0) {
        return OperationResult::Failure("存款金额必须为正数");
    }
    
    // 检查金额是否为100的倍数
    if (std::fmod(amount, 100) != 0) {
        return OperationResult::Failure("存款金额必须为100的倍数");
    }
    
    // 查找账户
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("账户不存在");
    }
    
    const Account& account = accountOpt.value();
    
    // 检查账户是否被锁定
    if (account.isLocked) {
        return OperationResult::Failure("账户已锁定");
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
    // 验证卡号
    if (fromCardNumber.isEmpty()) {
        return OperationResult::Failure("源卡号不能为空");
    }
    
    if (toCardNumber.isEmpty()) {
        return OperationResult::Failure("目标卡号不能为空");
    }
    
    // 验证源卡号和目标卡号不能相同
    if (fromCardNumber == toCardNumber) {
        return OperationResult::Failure("源卡号和目标卡号不能相同");
    }
    
    // 验证金额
    if (amount <= 0) {
        return OperationResult::Failure("转账金额必须为正数");
    }
    
    // 查找源账户
    std::optional<Account> fromAccountOpt = m_repository->findByCardNumber(fromCardNumber);
    if (!fromAccountOpt) {
        return OperationResult::Failure("源账户不存在");
    }
    
    const Account& fromAccount = fromAccountOpt.value();
    
    // 检查源账户是否被锁定
    if (fromAccount.isLocked) {
        return OperationResult::Failure("源账户已锁定");
    }
    
    // 查找目标账户
    std::optional<Account> toAccountOpt = m_repository->findByCardNumber(toCardNumber);
    if (!toAccountOpt) {
        return OperationResult::Failure("目标账户不存在");
    }
    
    const Account& toAccount = toAccountOpt.value();
    
    // 检查目标账户是否被锁定
    if (toAccount.isLocked) {
        return OperationResult::Failure("目标账户已锁定");
    }
    
    // 检查源账户余额是否足够
    if (amount > fromAccount.balance) {
        return OperationResult::Failure("余额不足");
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
    // 如果提供了confirmPin，验证新PIN和确认PIN是否匹配
    if (!confirmPin.isEmpty() && newPin != confirmPin) {
        return OperationResult::Failure("新PIN码与确认PIN码不匹配");
    }
    
    // 验证账户是否存在
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("账户不存在");
    }
    
    const Account& account = accountOpt.value();
    
    // 检查账户是否被锁定
    if (account.isLocked) {
        return OperationResult::Failure("账户已锁定");
    }
    
    // 验证当前PIN是否正确
    if (account.pin != currentPin) {
        return OperationResult::Failure("当前PIN码不正确");
    }
    
    // 验证新PIN格式
    return validatePinFormat(newPin);
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
    // 验证PIN格式：长度4-6位
    if (pin.length() < 4 || pin.length() > 6) {
        return OperationResult::Failure("PIN码长度必须在4-6位之间");
    }
    
    // 检查PIN是否为纯数字
    bool isNumeric = true;
    for (QChar c : pin) {
        if (!c.isDigit()) {
            isNumeric = false;
            break;
        }
    }
    
    if (!isNumeric) {
        return OperationResult::Failure("PIN码只能包含数字");
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
    // 验证卡号长度为16位
    if (cardNumber.length() != 16) {
        return OperationResult::Failure("卡号必须为16位");
    }
    
    // 验证卡号是否为纯数字
    bool isNumeric = true;
    for (QChar c : cardNumber) {
        if (!c.isDigit()) {
            isNumeric = false;
            break;
        }
    }
    
    if (!isNumeric) {
        return OperationResult::Failure("卡号只能包含数字");
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