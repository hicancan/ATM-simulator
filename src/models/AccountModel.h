#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include "TransactionModel.h"

// Account data structure
struct Account {
    QString cardNumber;
    QString pin;
    QString holderName;
    double balance;
    double withdrawLimit;
    bool isLocked;
    bool isAdmin;
    
    // 转换为JSON对象
    QJsonObject toJson() const {
        QJsonObject json;
        json["cardNumber"] = cardNumber;
        json["pin"] = pin;
        json["holderName"] = holderName;
        json["balance"] = balance;
        json["withdrawLimit"] = withdrawLimit;
        json["isLocked"] = isLocked;
        json["isAdmin"] = isAdmin;
        return json;
    }
    
    // 从JSON对象创建Account
    static Account fromJson(const QJsonObject &json) {
        Account account;
        account.cardNumber = json["cardNumber"].toString();
        account.pin = json["pin"].toString();
        account.holderName = json["holderName"].toString();
        account.balance = json["balance"].toDouble();
        account.withdrawLimit = json["withdrawLimit"].toDouble();
        account.isLocked = json["isLocked"].toBool();
        account.isAdmin = json.contains("isAdmin") ? json["isAdmin"].toBool() : false;
        return account;
    }
};

// 结果类型，用于返回操作结果和错误信息
struct OperationResult {
    bool success;
    QString errorMessage;
    
    static OperationResult Success() { return {true, ""}; }
    static OperationResult Failure(const QString &error) { return {false, error}; }
};

class AccountModel : public QObject
{
    Q_OBJECT

public:
    explicit AccountModel(QObject *parent = nullptr);
    ~AccountModel();
    
    bool validateCredentials(const QString &cardNumber, const QString &pin);
    bool withdrawAmount(const QString &cardNumber, double amount);
    bool depositAmount(const QString &cardNumber, double amount);
    bool transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount);
    bool accountExists(const QString &cardNumber) const;
    QString getAccountHolderName(const QString &cardNumber) const;
    double getBalance(const QString &cardNumber) const;
    QString getHolderName(const QString &cardNumber) const;
    double getWithdrawLimit(const QString &cardNumber) const;
    bool isAccountLocked(const QString &cardNumber) const;
    bool changePin(const QString &cardNumber, const QString &oldPin, const QString &newPin);
    
    // Balance prediction
    double predictBalance(const QString &cardNumber, const TransactionModel* transactionModel, int daysInFuture = 7) const;
    
    // 管理员相关方法
    bool isAdmin(const QString &cardNumber) const;
    QVector<Account> getAllAccounts() const;
    bool createAccount(const Account &account);
    bool updateAccount(const Account &account);
    bool deleteAccount(const QString &cardNumber);
    bool setAccountLockStatus(const QString &cardNumber, bool locked);
    bool setWithdrawLimit(const QString &cardNumber, double limit);
    
    // For demonstration, these methods help manage the in-memory data
    void lockAccount(const QString &cardNumber, bool locked);
    void addAccount(const Account &account);
    
    // 持久化存储方法
    bool saveAccounts(const QString &filename = "accounts.json");
    bool loadAccounts(const QString &filename = "accounts.json");
    
    // 新增的业务逻辑方法，返回 OperationResult
    OperationResult validateLogin(const QString &cardNumber, const QString &pin);
    OperationResult validateAdminLogin(const QString &cardNumber, const QString &pin);
    OperationResult validateWithdrawal(const QString &cardNumber, double amount);
    OperationResult validateDeposit(const QString &cardNumber, double amount);
    OperationResult validateTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount);
    OperationResult validatePinChange(const QString &cardNumber, const QString &currentPin, const QString &newPin, const QString &confirmPin);
    OperationResult validateCreateAccount(const QString &cardNumber, const QString &pin, const QString &holderName, 
                                      double balance, double withdrawLimit, bool isAdmin);
    OperationResult validateUpdateAccount(const QString &cardNumber, const QString &holderName, 
                                      double balance, double withdrawLimit);
    OperationResult performWithdrawal(const QString &cardNumber, double amount);
    OperationResult performDeposit(const QString &cardNumber, double amount);
    OperationResult performTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount);
    QString getTargetAccountInfo(const QString &cardNumber, const QString &targetCardNumber);
    
    // 账户数据转换方法（将Account转换为ViewModel可用的格式）
    QVariantMap accountToVariantMap(const Account &account) const;
    QVariantList getAllAccountsAsVariantList() const;
    
    // 执行重置PIN的方法
    OperationResult resetPin(const QString &cardNumber, const QString &newPin);
    
    // 从现有账户保留PIN和管理员状态，更新其他字段
    OperationResult updateAccountFromViewModel(const QString &cardNumber, const QString &holderName, 
                                           double balance, double withdrawLimit, bool isLocked);
    
    // 新增权限验证方法
    OperationResult validateAdminOperation(const QString &cardNumber);

    // 新增完整的预测余额方法，包含所有验证逻辑
    OperationResult calculatePredictedBalance(const QString &cardNumber, 
                                            const TransactionModel* transactionModel, 
                                            int daysInFuture, 
                                            double &outBalance);

    // 新增完整的业务处理方法，包含所有验证和操作逻辑
    struct LoginResult {
        bool success;
        QString errorMessage;
        bool isAdmin;
        QString holderName;
        double balance;
        double withdrawLimit;
    };
    
    // 完整的登录处理
    LoginResult performLogin(const QString &cardNumber, const QString &pin);
    LoginResult performAdminLogin(const QString &cardNumber, const QString &pin);
    
    // 完整的密码修改处理
    OperationResult performPinChange(const QString &cardNumber, const QString &currentPin, const QString &newPin, const QString &confirmPin);
    
    // 验证目标账户（转账时）
    OperationResult validateTargetAccount(const QString &sourceCard, const QString &targetCard);

private:
    // In a real application, this would be replaced with database access
    QMap<QString, Account> m_accounts;
    
    // Initialize with some test accounts
    void initializeTestAccounts();
    
    // Helper method to find account
    Account* findAccount(const QString &cardNumber);
    const Account* findAccount(const QString &cardNumber) const;
    
    QString m_dataPath;
};