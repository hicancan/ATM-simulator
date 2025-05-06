#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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