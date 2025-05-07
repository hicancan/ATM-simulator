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
    // 清除错误消息
    clearError();
    
    // 使用Model层处理完整的登录逻辑
    AccountModel::LoginResult result = m_accountModel.performLogin(m_cardNumber, pin);
    
    if (result.success) {
        // 更新视图状态
        m_isLoggedIn = true;
        m_isAdmin = result.isAdmin;
        
        // 发送信号通知UI更新
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        emit isAdminChanged();
        
        // 记录登录交易
        recordTransaction(TransactionType::Other, 0.0, balance(), "登录成功");
        return true;
    } else {
        // 处理错误
        setErrorMessage(result.errorMessage);
        return false;
    }
}

bool AccountViewModel::loginWithCard(const QString &cardNumber, const QString &pin)
{
    // 清除错误消息
    clearError();
    
    // 特殊处理管理员登录
    if (cardNumber == "9999888877776666") {
        AccountModel::LoginResult result = m_accountModel.performAdminLogin(cardNumber, pin);
        if (result.success) {
            // 设置当前卡号
            m_cardNumber = cardNumber;
            emit cardNumberChanged();
            
            // 更新视图状态
            m_isLoggedIn = true;
            m_isAdmin = true;
            
            // 发送信号通知UI更新
            emit isLoggedInChanged();
            emit holderNameChanged();
            emit balanceChanged();
            emit withdrawLimitChanged();
            emit isAdminChanged();
            
            // 记录登录交易
            recordTransaction(TransactionType::Other, 0.0, balance(), "管理员登录成功");
            return true;
        } else {
            setErrorMessage(result.errorMessage);
            return false;
        }
    }
    
    // 处理普通账户登录
    AccountModel::LoginResult result = m_accountModel.performLogin(cardNumber, pin);
    if (result.success) {
        // 设置当前卡号
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        
        // 更新视图状态
        m_isLoggedIn = true;
        m_isAdmin = result.isAdmin;
        
        // 发送信号通知UI更新
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        emit isAdminChanged();
        
        // 记录登录交易
        recordTransaction(TransactionType::Other, 0.0, balance(), "登录成功");
        return true;
    } else {
        setErrorMessage(result.errorMessage);
        return false;
    }
}

bool AccountViewModel::withdraw(double amount)
{
    clearError();
    
    // 检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    // 使用 Model 层验证取款
    OperationResult result = m_accountModel.validateWithdrawal(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }
    
    // 执行取款操作
    result = m_accountModel.performWithdrawal(m_cardNumber, amount);
    if (result.success) {
        emit balanceChanged();
        
        // 记录交易
        recordTransaction(TransactionType::Withdrawal, amount, balance(), "取款成功");
        
        emit transactionCompleted(true, QString("成功取款：￥%1").arg(amount));
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "取款失败，请重试" : result.errorMessage);
        return false;
    }
}

bool AccountViewModel::deposit(double amount)
{
    clearError();
    
    // 检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    // 使用 Model 层验证存款
    OperationResult result = m_accountModel.validateDeposit(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }
    
    // 执行存款操作
    result = m_accountModel.performDeposit(m_cardNumber, amount);
    if (result.success) {
        emit balanceChanged();
        
        // 记录交易
        recordTransaction(TransactionType::Deposit, amount, balance(), "存款成功");
        
        emit transactionCompleted(true, QString("成功存款：￥%1").arg(amount));
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "存款失败，请重试" : result.errorMessage);
        return false;
    }
}

bool AccountViewModel::transfer(const QString &targetCard, double amount)
{
    clearError();
    
    // 检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    // 使用 Model 层验证转账
    OperationResult result = m_accountModel.validateTransfer(m_cardNumber, targetCard, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }
    
    // 执行转账操作
    result = m_accountModel.performTransfer(m_cardNumber, targetCard, amount);
    if (result.success) {
        emit balanceChanged();
        
        // 获取接收方姓名用于交易描述
        QString receiverName = m_accountModel.getHolderName(targetCard);
        QString description = QString("转账至%1（%2）").arg(receiverName).arg(targetCard.right(4));
        
        // 记录转账方的交易
        recordTransaction(TransactionType::Transfer, amount, balance(), description, targetCard);
        
        // 记录收款方的交易 - 使用 Model 层的方法
        double receiverBalance = m_accountModel.getBalance(targetCard);
        m_transactionModel->recordTransferReceipt(m_cardNumber, holderName(), targetCard, amount, receiverBalance);
        
        emit transactionCompleted(true, QString("成功转账：￥%1 至 %2").arg(amount).arg(receiverName));
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "转账失败，请重试" : result.errorMessage);
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
    
    // 使用Model层专门验证目标账户的方法
    OperationResult result = m_accountModel.validateTargetAccount(m_cardNumber, targetCard);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }
    
    return true;
}

QString AccountViewModel::getTargetCardHolderName(const QString &targetCard)
{
    return m_accountModel.getTargetAccountInfo(m_cardNumber, targetCard);
}

bool AccountViewModel::changePassword(const QString &currentPin, const QString &newPin, const QString &confirmPin)
{
    clearError();
    
    // 检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }
    
    // 使用 Model 层处理完整的密码修改逻辑
    OperationResult result = m_accountModel.performPinChange(m_cardNumber, currentPin, newPin, confirmPin);
    if (result.success) {
        // 记录交易
        recordTransaction(TransactionType::Other, 0.0, balance(), "修改PIN码成功");
        
        // 通知UI
        emit transactionCompleted(true, "PIN码修改成功");
        return true;
    } else {
        setErrorMessage(result.errorMessage);
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
        // 直接使用 TransactionModel 中的方法记录交易
        m_transactionModel->recordTransaction(m_cardNumber, type, amount, balanceAfter, description, targetCard);
        
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
    // 初始化预测余额变量
    double newPredictedBalance = 0.0;
    
    // 使用Model层验证和计算预测余额
    OperationResult result = m_accountModel.calculatePredictedBalance(
        m_cardNumber, m_transactionModel, daysInFuture, newPredictedBalance);
    
    // 如果计算失败，输出警告并可能重置预测余额
    if (!result.success) {
        qWarning() << "预测余额计算失败:" << result.errorMessage;
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
        return;
    }

    // 更新余额并发送通知
    if (m_predictedBalance != newPredictedBalance) {
        m_predictedBalance = newPredictedBalance;
        emit predictedBalanceChanged();
    }
}

QVariantList AccountViewModel::getAllAccounts()
{
    // 只允许管理员获取所有账户信息
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return QVariantList();
    }
    
    // 直接使用 Model 层的方法获取账户列表
    return m_accountModel.getAllAccountsAsVariantList();
}

bool AccountViewModel::createAccount(const QString &cardNumber, const QString &pin, const QString &holderName, 
                                  double balance, double withdrawLimit, bool isLocked, bool isAdmin)
{
    // 检查权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 使用 Model 层验证创建账户
    OperationResult result = m_accountModel.validateCreateAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
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
    // 检查权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }
    
    // 使用 Model 层的方法直接更新账户
    OperationResult result = m_accountModel.updateAccountFromViewModel(cardNumber, holderName, balance, withdrawLimit, isLocked);
    if (result.success) {
        emit accountsChanged();
        
        // 如果更新的是当前登录用户，刷新数据
        if (cardNumber == m_cardNumber) {
            emit holderNameChanged();
            emit balanceChanged();
            emit withdrawLimitChanged();
        }
        
        return true;
    } else {
        setErrorMessage(result.errorMessage);
        return false;
    }
}

bool AccountViewModel::deleteAccount(const QString &cardNumber)
{
    clearError();
    
    // 使用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
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
    
    // 使用 Model 层的 resetPin 方法
    OperationResult result = m_accountModel.resetPin(cardNumber, newPin);
    if (result.success) {
        emit accountsChanged();
        return true;
    } else {
        setErrorMessage(result.errorMessage);
        return false;
    }
}

bool AccountViewModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    clearError();
    
    // 使用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }
    
    // 不能锁定自己的账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能锁定当前登录账户");
        return false;
    }
    
    // 验证输入
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
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
    clearError();
    
    // 使用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }
    
    // 验证限额是否合理
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