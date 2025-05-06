#include "AccountViewModel.h"
#include <QDateTime>
#include <QDebug>

AccountViewModel::AccountViewModel(QObject *parent)
    : QObject(parent)
    , m_transactionModel(nullptr)
    , m_isLoggedIn(false)
    , m_predictedBalance(0.0)
    , m_isAdmin(false)
{
}

void AccountViewModel::setTransactionModel(TransactionModel *model)
{
    m_transactionModel = model;
}

QString AccountViewModel::cardNumber() const
{
    return m_cardNumber;
}

void AccountViewModel::setCardNumber(const QString &cardNumber)
{
    if (m_cardNumber != cardNumber) {
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        
        // Reset predicted balance when card number changes
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
    }
}

QString AccountViewModel::holderName() const
{
    return m_isLoggedIn ? m_accountModel.getHolderName(m_cardNumber) : QString();
}

double AccountViewModel::balance() const
{
    return m_isLoggedIn ? m_accountModel.getBalance(m_cardNumber) : 0.0;
}

double AccountViewModel::withdrawLimit() const
{
    return m_isLoggedIn ? m_accountModel.getWithdrawLimit(m_cardNumber) : 0.0;
}

bool AccountViewModel::isLoggedIn() const
{
    return m_isLoggedIn;
}

QString AccountViewModel::errorMessage() const
{
    return m_errorMessage;
}

bool AccountViewModel::isAdmin() const
{
    return m_isAdmin;
}

bool AccountViewModel::login(const QString &pin)
{
    // Always clear error message before attempting login
    m_errorMessage.clear();
    emit errorMessageChanged();
    
    if (m_cardNumber.isEmpty()) {
        setErrorMessage("请输入卡号");
        return false;
    }
    
    if (pin.isEmpty()) {
        setErrorMessage("请输入PIN码");
        return false;
    }
    
    if (m_accountModel.validateCredentials(m_cardNumber, pin)) {
        m_isLoggedIn = true;
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        
        // Record login transaction
        recordTransaction(TransactionType::Other, 0.0, balance(), "登录成功");
        return true;
    } else {
        if (m_accountModel.isAccountLocked(m_cardNumber)) {
            setErrorMessage("账户已锁定，请联系银行客服");
        } else {
            setErrorMessage("卡号或PIN码错误");
        }
        return false;
    }
}

bool AccountViewModel::loginWithCard(const QString &cardNumber, const QString &pin)
{
    // Always clear error message before attempting login
    m_errorMessage.clear();
    emit errorMessageChanged();
    
    if (cardNumber.isEmpty()) {
        setErrorMessage("请输入卡号");
        return false;
    }
    
    if (pin.isEmpty()) {
        setErrorMessage("请输入PIN码");
        return false;
    }
    
    qDebug() << "尝试登录 - 卡号:" << cardNumber << "PIN:" << pin;
    
    // 特殊处理管理员账户
    if (cardNumber == "9999888877776666" && pin == "8888") {
        qDebug() << "管理员账户直接验证通过";
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        
        m_isLoggedIn = true;
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        
        // 设置管理员标志
        m_isAdmin = true;
        emit isAdminChanged();
        
        // 记录登录交易
        recordTransaction(TransactionType::Other, 0.0, balance(), "管理员登录成功");
        
        qDebug() << "管理员登录成功，卡号:" << m_cardNumber;
        return true;
    }
    
    // 处理普通账户
    if (m_accountModel.validateCredentials(cardNumber, pin)) {
        // 验证成功后再设置当前卡号
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        
        m_isLoggedIn = true;
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        
        // Record login transaction
        recordTransaction(TransactionType::Other, 0.0, balance(), "登录成功");
        
        // 检查是否为管理员账户
        m_isAdmin = m_accountModel.isAdmin(cardNumber);
        emit isAdminChanged();
        
        qDebug() << "登录成功，卡号:" << m_cardNumber << "，用户类型:" << (m_isAdmin ? "管理员" : "普通用户");
        return true;
    } else {
        // 验证失败，使用传入的cardNumber检查锁定状态
        if (m_accountModel.isAccountLocked(cardNumber)) {
            setErrorMessage("账户已锁定，请联系银行客服");
            qDebug() << "登录失败 - 账户已锁定:" << cardNumber;
        } else {
            setErrorMessage("卡号或PIN码错误");
            qDebug() << "登录失败 - 卡号或PIN码错误:" << cardNumber;
        }
        return false;
    }
}

bool AccountViewModel::withdraw(double amount)
{
    clearError();
    
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    if (amount <= 0) {
        setErrorMessage("请输入有效金额");
        return false;
    }
    
    if (amount > withdrawLimit()) {
        setErrorMessage(QString("超出取款限额 (%1)").arg(withdrawLimit()));
        return false;
    }
    
    if (amount > balance()) {
        setErrorMessage("余额不足");
        return false;
    }
    
    if (m_accountModel.withdrawAmount(m_cardNumber, amount)) {
        emit balanceChanged();
        
        // Record transaction
        recordTransaction(TransactionType::Withdrawal, amount, balance(), "取款成功");
        
        emit transactionCompleted(true, QString("成功取款：￥%1").arg(amount));
        return true;
    } else {
        setErrorMessage("取款失败，请重试");
        return false;
    }
}

bool AccountViewModel::deposit(double amount)
{
    clearError();
    
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    if (amount <= 0) {
        setErrorMessage("请输入有效金额");
        return false;
    }
    
    if (m_accountModel.depositAmount(m_cardNumber, amount)) {
        emit balanceChanged();
        
        // Record transaction
        recordTransaction(TransactionType::Deposit, amount, balance(), "存款成功");
        
        emit transactionCompleted(true, QString("成功存款：￥%1").arg(amount));
        return true;
    } else {
        setErrorMessage("存款失败，请重试");
        return false;
    }
}

bool AccountViewModel::transfer(const QString &targetCard, double amount)
{
    clearError();
    
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    if (targetCard.isEmpty()) {
        setErrorMessage("请输入目标卡号");
        return false;
    }
    
    if (targetCard == m_cardNumber) {
        setErrorMessage("不能转账给自己");
        return false;
    }
    
    if (!m_accountModel.accountExists(targetCard)) {
        setErrorMessage("目标账户不存在");
        return false;
    }
    
    if (amount <= 0) {
        setErrorMessage("请输入有效金额");
        return false;
    }
    
    if (amount > balance()) {
        setErrorMessage("余额不足");
        return false;
    }
    
    if (m_accountModel.transferAmount(m_cardNumber, targetCard, amount)) {
        emit balanceChanged();
        
        // 获取接收方姓名用于交易描述
        QString receiverName = m_accountModel.getHolderName(targetCard);
        QString description = QString("转账至%1（%2）").arg(receiverName).arg(targetCard.right(4));
        
        // 记录交易
        recordTransaction(TransactionType::Transfer, amount, balance(), description, targetCard);
        
        emit transactionCompleted(true, QString("成功转账：￥%1 至 %2").arg(amount).arg(receiverName));
        return true;
    } else {
        setErrorMessage("转账失败，请重试");
        return false;
    }
}

bool AccountViewModel::validateTargetCard(const QString &targetCard)
{
    clearError();
    
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    if (targetCard.isEmpty()) {
        setErrorMessage("请输入目标卡号");
        return false;
    }
    
    if (targetCard == m_cardNumber) {
        setErrorMessage("不能转账给自己");
        return false;
    }
    
    if (!m_accountModel.accountExists(targetCard)) {
        setErrorMessage("目标账户不存在");
        return false;
    }
    
    return true;
}

QString AccountViewModel::getTargetCardHolderName(const QString &targetCard)
{
    return m_accountModel.getAccountHolderName(targetCard);
}

bool AccountViewModel::changePassword(const QString &currentPin, const QString &newPin, const QString &confirmPin)
{
    clearError();
    
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    if (currentPin.isEmpty()) {
        setErrorMessage("请输入当前PIN码");
        return false;
    }
    
    if (newPin.isEmpty()) {
        setErrorMessage("请输入新PIN码");
        return false;
    }
    
    if (confirmPin.isEmpty()) {
        setErrorMessage("请再次输入新PIN码");
        return false;
    }
    
    if (newPin != confirmPin) {
        setErrorMessage("两次输入的新PIN码不匹配");
        return false;
    }
    
    if (newPin.length() != 4) {
        setErrorMessage("PIN码必须是4位数字");
        return false;
    }
    
    bool isPinValid = false;
    newPin.toInt(&isPinValid);
    if (!isPinValid) {
        setErrorMessage("PIN码必须是数字");
        return false;
    }
    
    if (currentPin == newPin) {
        setErrorMessage("新PIN码不能与当前PIN码相同");
        return false;
    }
    
    if (m_accountModel.changePin(m_cardNumber, currentPin, newPin)) {
        // Record transaction
        recordTransaction(TransactionType::Other, 0.0, balance(), "修改PIN码成功");
        
        emit transactionCompleted(true, "PIN码修改成功");
        return true;
    } else {
        setErrorMessage("PIN码修改失败，当前PIN码不正确");
        return false;
    }
}

void AccountViewModel::logout()
{
    if (m_isLoggedIn) {
        m_isLoggedIn = false;
        m_isAdmin = false;
        m_cardNumber.clear();
        m_errorMessage.clear();
        
        // Record logout transaction
        recordTransaction(TransactionType::Other, 0.0, balance(), "登出系统");
        
        emit isLoggedInChanged();
        emit isAdminChanged();
        emit cardNumberChanged();
        emit errorMessageChanged();
        emit loggedOut();
        
        qDebug() << "成功登出系统";
        
        // Reset predicted balance on logout
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
    }
}

void AccountViewModel::clearError()
{
    if (!m_errorMessage.isEmpty()) {
        m_errorMessage.clear();
        emit errorMessageChanged();
    }
}

void AccountViewModel::recordTransaction(TransactionType type, double amount, double balanceAfter, const QString &description, const QString &targetCard)
{
    if (m_transactionModel) {
        Transaction transaction;
        transaction.cardNumber = m_cardNumber;
        transaction.timestamp = QDateTime::currentDateTime();
        
        // 直接赋值，不需要switch转换
        transaction.type = type;
        transaction.amount = amount;
        transaction.balanceAfter = balanceAfter;
        transaction.description = description;
        transaction.targetCardNumber = targetCard;
        
        m_transactionModel->addTransaction(transaction);
        
        // 确保交易视图模型得到更新
        emit transactionRecorded();
    }
}

void AccountViewModel::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
    emit errorMessageChanged();
}

double AccountViewModel::predictedBalance() const
{
    return m_predictedBalance;
}

void AccountViewModel::calculatePredictedBalance(int daysInFuture)
{
    if (!m_isLoggedIn || m_cardNumber.isEmpty()) {
        qWarning() << "User not logged in or card number is empty. Cannot calculate predicted balance.";
        if (m_predictedBalance != 0.0) { // Reset if previously set
             m_predictedBalance = 0.0;
             emit predictedBalanceChanged();
        }
        return;
    }
    
    if (!m_transactionModel) {
        qWarning() << "TransactionModel is not set in AccountViewModel.";
        if (m_predictedBalance != 0.0) { // Reset if previously set
             m_predictedBalance = 0.0;
             emit predictedBalanceChanged();
        }
        return;
    }

    double newPredictedBalance = m_accountModel.predictBalance(m_cardNumber, m_transactionModel, daysInFuture);
    if (m_predictedBalance != newPredictedBalance) {
        m_predictedBalance = newPredictedBalance;
        emit predictedBalanceChanged();
    }
}

QVariantList AccountViewModel::getAllAccounts()
{
    QVariantList result;
    
    // 只允许管理员获取所有账户信息
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return result;
    }
    
    QVector<Account> accounts = m_accountModel.getAllAccounts();
    for (const Account &account : accounts) {
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

bool AccountViewModel::createAccount(const QString &cardNumber, const QString &pin, const QString &holderName, 
                                  double balance, double withdrawLimit, bool isLocked, bool isAdmin)
{
    // 只允许管理员创建账户
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 验证输入
    if (cardNumber.length() != 16 || !cardNumber.toULongLong()) {
        setErrorMessage("卡号必须是16位数字");
        return false;
    }
    
    if (pin.length() != 4 || !pin.toInt()) {
        setErrorMessage("PIN码必须是4位数字");
        return false;
    }
    
    if (holderName.isEmpty()) {
        setErrorMessage("持卡人姓名不能为空");
        return false;
    }
    
    if (balance < 0) {
        setErrorMessage("余额不能为负数");
        return false;
    }
    
    if (withdrawLimit < 0) {
        setErrorMessage("取款限额不能为负数");
        return false;
    }
    
    // 创建账户对象
    Account account;
    account.cardNumber = cardNumber;
    account.pin = pin;
    account.holderName = holderName;
    account.balance = balance;
    account.withdrawLimit = withdrawLimit;
    account.isLocked = isLocked;
    account.isAdmin = isAdmin;
    
    // 添加账户
    bool success = m_accountModel.createAccount(account);
    if (success) {
        emit accountsChanged();
    } else {
        setErrorMessage("创建账户失败，可能卡号已存在");
    }
    
    return success;
}

bool AccountViewModel::updateAccount(const QString &cardNumber, const QString &holderName, 
                                  double balance, double withdrawLimit, bool isLocked)
{
    // 只允许管理员更新账户
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 验证输入
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
        return false;
    }
    
    if (holderName.isEmpty()) {
        setErrorMessage("持卡人姓名不能为空");
        return false;
    }
    
    if (balance < 0) {
        setErrorMessage("余额不能为负数");
        return false;
    }
    
    if (withdrawLimit < 0) {
        setErrorMessage("取款限额不能为负数");
        return false;
    }
    
    // 获取现有账户数据，以保留PIN和管理员状态
    QVector<Account> accounts = m_accountModel.getAllAccounts();
    Account account;
    bool found = false;
    
    for (const Account &acc : accounts) {
        if (acc.cardNumber == cardNumber) {
            account = acc;
            found = true;
            break;
        }
    }
    
    if (!found) {
        setErrorMessage("找不到账户");
        return false;
    }
    
    // 更新可以修改的字段
    account.holderName = holderName;
    account.balance = balance;
    account.withdrawLimit = withdrawLimit;
    account.isLocked = isLocked;
    
    // 更新账户
    bool success = m_accountModel.updateAccount(account);
    if (success) {
        emit accountsChanged();
        
        // 如果更新的是当前登录用户，刷新数据
        if (cardNumber == m_cardNumber) {
            emit holderNameChanged();
            emit balanceChanged();
            emit withdrawLimitChanged();
        }
    } else {
        setErrorMessage("更新账户失败");
    }
    
    return success;
}

bool AccountViewModel::deleteAccount(const QString &cardNumber)
{
    // 只允许管理员删除账户
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 不能删除自己的账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能删除当前登录账户");
        return false;
    }
    
    // 删除账户
    bool success = m_accountModel.deleteAccount(cardNumber);
    if (success) {
        emit accountsChanged();
    } else {
        setErrorMessage("删除账户失败，可能卡号不存在");
    }
    
    return success;
}

bool AccountViewModel::resetAccountPin(const QString &cardNumber, const QString &newPin)
{
    // 只允许管理员重置PIN
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 验证输入
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
        return false;
    }
    
    if (newPin.length() != 4 || !newPin.toInt()) {
        setErrorMessage("PIN码必须是4位数字");
        return false;
    }
    
    // 获取现有账户
    QVector<Account> accounts = m_accountModel.getAllAccounts();
    Account account;
    bool found = false;
    
    for (const Account &acc : accounts) {
        if (acc.cardNumber == cardNumber) {
            account = acc;
            found = true;
            break;
        }
    }
    
    if (!found) {
        setErrorMessage("找不到账户");
        return false;
    }
    
    // 更新PIN
    account.pin = newPin;
    
    // 更新账户
    bool success = m_accountModel.updateAccount(account);
    if (success) {
        emit accountsChanged();
    } else {
        setErrorMessage("重置PIN码失败");
    }
    
    return success;
}

bool AccountViewModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    // 只允许管理员设置账户锁定状态
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 验证输入
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
        return false;
    }
    
    // 不能锁定自己的账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能锁定当前登录账户");
        return false;
    }
    
    // 设置锁定状态
    bool success = m_accountModel.setAccountLockStatus(cardNumber, locked);
    if (success) {
        emit accountsChanged();
    } else {
        setErrorMessage(locked ? "锁定账户失败" : "解锁账户失败");
    }
    
    return success;
}

bool AccountViewModel::setWithdrawLimit(const QString &cardNumber, double limit)
{
    // 只允许管理员设置取款限额
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 验证输入
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
        return false;
    }
    
    if (limit < 0) {
        setErrorMessage("取款限额不能为负数");
        return false;
    }
    
    // 设置取款限额
    bool success = m_accountModel.setWithdrawLimit(cardNumber, limit);
    if (success) {
        emit accountsChanged();
        
        // 如果更新的是当前登录用户，刷新数据
        if (cardNumber == m_cardNumber) {
            emit withdrawLimitChanged();
        }
    } else {
        setErrorMessage("设置取款限额失败");
    }
    
    return success;
} 