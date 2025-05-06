#include "AccountViewModel.h"
#include <QDateTime>

AccountViewModel::AccountViewModel(QObject *parent)
    : QObject(parent)
    , m_transactionModel(nullptr)
    , m_isLoggedIn(false)
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
    
    // Set card number directly
    m_cardNumber = cardNumber;
    emit cardNumberChanged();
    
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
        
        // Record logout transaction
        recordTransaction(TransactionType::Other, 0.0, balance(), "登出系统");
        
        emit isLoggedInChanged();
        emit loggedOut();
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