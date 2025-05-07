#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// Transaction type enum
enum class TransactionType {
    Deposit,
    Withdrawal,
    BalanceInquiry,
    Transfer,
    Other
};

// Transaction data structure
struct Transaction {
    QString cardNumber;
    QDateTime timestamp;
    TransactionType type;
    double amount;
    double balanceAfter;
    QString description;
    QString targetCardNumber; // 用于转账时记录目标账户
    
    // 转换为JSON对象
    QJsonObject toJson() const {
        QJsonObject json;
        json["cardNumber"] = cardNumber;
        json["timestamp"] = timestamp.toString(Qt::ISODate);
        json["type"] = static_cast<int>(type);
        json["amount"] = amount;
        json["balanceAfter"] = balanceAfter;
        json["description"] = description;
        json["targetCardNumber"] = targetCardNumber;
        return json;
    }
    
    // 从JSON对象创建Transaction
    static Transaction fromJson(const QJsonObject &json) {
        Transaction transaction;
        transaction.cardNumber = json["cardNumber"].toString();
        transaction.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
        transaction.type = static_cast<TransactionType>(json["type"].toInt());
        transaction.amount = json["amount"].toDouble();
        transaction.balanceAfter = json["balanceAfter"].toDouble();
        transaction.description = json["description"].toString();
        transaction.targetCardNumber = json["targetCardNumber"].toString();
        return transaction;
    }
};

class TransactionModel : public QObject
{
    Q_OBJECT

public:
    explicit TransactionModel(QObject *parent = nullptr);
    ~TransactionModel();

    // Core transaction operations
    void addTransaction(const Transaction &transaction);
    QVector<Transaction> getTransactionsForCard(const QString &cardNumber) const;
    QVector<Transaction> getRecentTransactions(const QString &cardNumber, int count) const;
    
    // 新增的业务逻辑方法，用于创建交易记录
    Transaction createTransaction(const QString &cardNumber, TransactionType type, 
                                double amount, double balanceAfter, 
                                const QString &description, const QString &targetCard = QString());
    
    // 创建交易记录并添加到模型中
    void recordTransaction(const QString &cardNumber, TransactionType type, 
                          double amount, double balanceAfter, 
                          const QString &description, const QString &targetCard = QString());
    
    // 创建转账目标账户的交易记录（存款类型）
    void recordTransferReceipt(const QString &fromCardNumber, const QString &fromCardHolderName, 
                              const QString &toCardNumber, double amount, double balanceAfter);
    
    // Clear transactions for a specific card (e.g., on logout)
    void clearTransactionsForCard(const QString &cardNumber);
    
    // 持久化存储方法
    bool saveTransactions(const QString &filename = "transactions.json");
    bool loadTransactions(const QString &filename = "transactions.json");

private:
    // 初始化测试交易数据
    void initializeTestTransactions();
    
    // In-memory storage for transactions
    QVector<Transaction> m_transactions;
    
    // 数据存储路径
    QString m_dataPath;
};