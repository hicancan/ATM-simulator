// TransactionModel.h
/**
 * @file TransactionModel.h
 * @brief 交易数据模型头文件
 *
 * 定义了交易结构体和交易数据管理类 TransactionModel。
 * TransactionModel 负责交易数据的存储、加载、检索和格式化。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVector>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale> // 用于格式化货币/数字
#include "JsonPersistenceManager.h"

/**
 * @brief 交易类型枚举
 */
enum class TransactionType {
    Deposit,        //!< 存款
    Withdrawal,     //!< 取款
    BalanceInquiry, //!< 余额查询
    Transfer,       //!< 转账
    Other           //!< 其他类型交易 (例如：登录、登出、PIN 码修改)
};

/**
 * @brief 交易数据结构体
 *
 * 存储单条交易的详细信息。
 */
struct Transaction {
    QString cardNumber;     //!< 交易涉及的卡号
    QDateTime timestamp;    //!< 交易发生的时间戳
    TransactionType type;   //!< 交易类型
    double amount;          //!< 交易金额
    double balanceAfter;    //!< 交易后的账户余额
    QString description;    //!< 交易描述
    QString targetCardNumber; //!< 目标卡号 (转账时记录对方卡号)

    /**
     * @brief 将 Transaction 对象转换为 QJsonObject
     * @return 包含交易数据的 QJsonObject
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["cardNumber"] = cardNumber;
        json["timestamp"] = timestamp.toString(Qt::ISODate); // 使用 ISO 格式以便可靠解析
        json["type"] = static_cast<int>(type);
        json["amount"] = amount;
        json["balanceAfter"] = balanceAfter;
        json["description"] = description;
        json["targetCardNumber"] = targetCardNumber;
        return json;
    }

    /**
     * @brief 从 QJsonObject 创建 Transaction 对象
     * @param json 包含交易数据的 QJsonObject
     * @return 创建的 Transaction 对象
     */
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

/**
 * @brief 交易数据模型类
 *
 * 负责管理交易数据，包括数据的加载、保存、记录和检索。
 * 提供数据格式化方法，但不直接与 UI 交互。
 */
class TransactionModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param persistenceManager JSON持久化管理器
     * @param filename 交易数据文件名，默认为"transactions.json"
     * @param parent 父对象
     */
    explicit TransactionModel(JsonPersistenceManager* persistenceManager,
                            const QString& filename = "transactions.json",
                            QObject *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~TransactionModel();

    // --- 核心交易操作 ---
    /**
     * @brief 添加交易记录到内存列表并保存
     * @param transaction 要添加的 Transaction 对象
     */
    void addTransaction(const Transaction &transaction);
    /**
     * @brief 获取指定卡号的所有交易记录
     * @param cardNumber 卡号
     * @return 包含该卡号所有交易记录的 QVector
     */
    QVector<Transaction> getTransactionsForCard(const QString &cardNumber) const;
    /**
     * @brief 获取指定卡号的最近交易记录
     * @param cardNumber 卡号
     * @param count 要获取的记录数量
     * @return 包含指定数量最近交易记录的 QVector
     */
    QVector<Transaction> getRecentTransactions(const QString &cardNumber, int count) const;

    // --- 交易创建和记录 ---
    /**
     * @brief 创建一个 Transaction 对象
     * @param cardNumber 卡号
     * @param type 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param description 交易描述
     * @param targetCard 目标卡号 (转账时使用)
     * @return 创建的 Transaction 对象
     */
    Transaction createTransaction(const QString &cardNumber, TransactionType type,
                                double amount, double balanceAfter,
                                const QString &description, const QString &targetCard = QString());
    /**
     * @brief 创建并记录交易
     *
     * 创建一个 Transaction 对象并添加到模型中。
     *
     * @param cardNumber 卡号
     * @param type 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param description 交易描述
     * @param targetCard 目标卡号 (转账时使用)
     */
    void recordTransaction(const QString &cardNumber, TransactionType type,
                          double amount, double balanceAfter,
                          const QString &description, const QString &targetCard = QString());
    /**
     * @brief 记录转账目标账户的交易记录（存款类型）
     *
     * 用于在转账成功后为收款方生成交易记录。
     *
     * @param fromCardNumber 源卡号
     * @param fromCardHolderName 源持卡人姓名
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @param balanceAfter 目标账户交易后余额
     */
    void recordTransferReceipt(const QString &fromCardNumber, const QString &fromCardHolderName,
                              const QString &toCardNumber, double amount, double balanceAfter);

    // --- 数据管理 ---
    /**
     * @brief 清除指定卡号的所有交易记录
     *
     * 例如在删除账户时使用。
     *
     * @param cardNumber 卡号
     */
    void clearTransactionsForCard(const QString &cardNumber);

    // --- 持久化存储方法 ---
    /**
     * @brief 保存交易记录到文件
     * @return 如果成功保存返回 true，否则返回 false
     */
    bool saveTransactions();
    /**
     * @brief 从文件加载交易记录
     * @return 如果成功加载返回 true，否则返回 false
     */
    bool loadTransactions();

    // --- 数据格式化方法 ---
    /**
     * @brief 格式化金额为货币字符串
     * @param amount 金额
     * @return 格式化后的金额字符串
     */
    QString formatAmount(double amount) const;
    /**
     * @brief 格式化日期时间为字符串
     * @param dateTime 日期时间对象
     * @return 格式化后的日期时间字符串
     */
    QString formatDate(const QDateTime &dateTime) const;
    /**
     * @brief 获取交易类型的显示名称
     * @param type 交易类型枚举的整型值
     * @return 交易类型的中文名称
     */
    QString getTransactionTypeName(int type) const;

private:
    /**
     * @brief 初始化测试交易数据
     *
     * 如果数据文件不存在，则创建一些预设的测试交易记录。
     */
    void initializeTestTransactions();

    //!< 交易记录内存存储
    QVector<Transaction> m_transactions;
    
    //!< JSON持久化管理器
    JsonPersistenceManager* m_persistenceManager;
    
    //!< 交易数据文件名
    QString m_filename;
    
    //!< 标记数据是否被修改
    bool m_isDirty;
};