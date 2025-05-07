#include "TransactionModel.h"
#include <algorithm>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFile>

TransactionModel::TransactionModel(QObject *parent)
    : QObject(parent)
{
    // 设置数据存储路径
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(m_dataPath);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    qDebug() << "交易记录存储路径:" << m_dataPath;
    
    // 尝试加载交易记录，如果加载失败则初始化测试交易
    if (!loadTransactions()) {
        qDebug() << "无法加载交易记录，初始化测试交易";
        initializeTestTransactions();
        saveTransactions(); // 保存初始化的测试交易
    }
}

TransactionModel::~TransactionModel()
{
    // 在析构函数中保存交易记录
    saveTransactions();
}

void TransactionModel::addTransaction(const Transaction &transaction)
{
    m_transactions.append(transaction);
    qDebug() << "新交易已添加: " << transaction.cardNumber 
             << "类型:" << static_cast<int>(transaction.type)
             << "金额:" << transaction.amount
             << "描述:" << transaction.description;
    
    // 添加新交易后保存数据
    saveTransactions();
}

QVector<Transaction> TransactionModel::getTransactionsForCard(const QString &cardNumber) const
{
    QVector<Transaction> result;
    
    for (const auto &transaction : m_transactions) {
        if (transaction.cardNumber == cardNumber) {
            result.append(transaction);
        }
    }
    
    qDebug() << "为卡号" << cardNumber << "找到" << result.size() << "条交易记录";
    return result;
}

QVector<Transaction> TransactionModel::getRecentTransactions(const QString &cardNumber, int count) const
{
    QVector<Transaction> transactions = getTransactionsForCard(cardNumber);
    
    // Sort by timestamp (newest first)
    std::sort(transactions.begin(), transactions.end(), 
              [](const Transaction &a, const Transaction &b) {
                  return a.timestamp > b.timestamp;
              });
    
    // Limit to requested count
    if (transactions.size() > count) {
        transactions.resize(count);
    }
    
    qDebug() << "返回" << transactions.size() << "条最近交易记录，请求数量为" << count;
    return transactions;
}

void TransactionModel::clearTransactionsForCard(const QString &cardNumber)
{
    int beforeSize = m_transactions.size();
    m_transactions.erase(
        std::remove_if(m_transactions.begin(), m_transactions.end(),
                      [&cardNumber](const Transaction &t) {
                          return t.cardNumber == cardNumber;
                      }),
        m_transactions.end());
    
    int removed = beforeSize - m_transactions.size();
    qDebug() << "已清除" << removed << "条交易记录，卡号: " << cardNumber;
    
    // 清除后保存数据
    saveTransactions();
}

// 实现保存交易记录的方法
bool TransactionModel::saveTransactions(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存交易记录:" << filePath << ", 错误:" << file.errorString();
        return false;
    }
    
    QJsonArray transactionsArray;
    
    // 将所有交易记录转换为JSON数组
    for (const auto &transaction : m_transactions) {
        transactionsArray.append(transaction.toJson());
    }
    
    QJsonDocument doc(transactionsArray);
    
    // 写入文件
    file.write(doc.toJson());
    file.close();
    
    qDebug() << "成功保存" << m_transactions.size() << "条交易记录到" << filePath;
    return true;
}

// 实现加载交易记录的方法
bool TransactionModel::loadTransactions(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);
    
    if (!file.exists()) {
        qWarning() << "交易记录文件不存在:" << filePath;
        return false;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件加载交易记录:" << filePath << ", 错误:" << file.errorString();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (doc.isNull() || !doc.isArray()) {
        qWarning() << "交易记录文件格式无效:" << filePath;
        return false;
    }
    
    // 清空当前交易记录列表
    m_transactions.clear();
    
    // 加载交易记录
    QJsonArray transactionsArray = doc.array();
    for (const QJsonValue &value : transactionsArray) {
        if (value.isObject()) {
            Transaction transaction = Transaction::fromJson(value.toObject());
            m_transactions.append(transaction);
        }
    }
    
    qDebug() << "成功从" << filePath << "加载" << m_transactions.size() << "条交易记录";
    return true;
}

// 添加初始化测试交易数据的方法
void TransactionModel::initializeTestTransactions()
{
    // 测试卡号，与AccountModel中的测试账户一致
    QString testCard1 = "1234567890123456"; // 张三
    QString testCard2 = "2345678901234567"; // 李四
    
    // 当前时间
    QDateTime now = QDateTime::currentDateTime();
    
    // 为张三添加一些测试交易
    // 1. 存款交易
    Transaction deposit1;
    deposit1.cardNumber = testCard1;
    deposit1.timestamp = now.addSecs(-5 * 24 * 3600); // 5天前
    deposit1.type = TransactionType::Deposit;
    deposit1.amount = 1000.0;
    deposit1.balanceAfter = 6000.0;
    deposit1.description = "ATM存款";
    m_transactions.append(deposit1);
    
    // 2. 取款交易
    Transaction withdraw1;
    withdraw1.cardNumber = testCard1;
    withdraw1.timestamp = now.addSecs(-3 * 24 * 3600); // 3天前
    withdraw1.type = TransactionType::Withdrawal;
    withdraw1.amount = 500.0;
    withdraw1.balanceAfter = 5500.0;
    withdraw1.description = "ATM取款";
    m_transactions.append(withdraw1);
    
    // 3. 转账交易
    Transaction transfer1;
    transfer1.cardNumber = testCard1;
    transfer1.timestamp = now.addSecs(-1 * 24 * 3600); // 1天前
    transfer1.type = TransactionType::Transfer;
    transfer1.amount = 500.0;
    transfer1.balanceAfter = 5000.0;
    transfer1.description = "转账至李四（4567）";
    transfer1.targetCardNumber = testCard2;
    m_transactions.append(transfer1);
    
    // 为李四添加一些测试交易
    // 1. 收到的转账
    Transaction transfer2;
    transfer2.cardNumber = testCard2;
    transfer2.timestamp = now.addSecs(-1 * 24 * 3600); // 1天前
    transfer2.type = TransactionType::Deposit;
    transfer2.amount = 500.0;
    transfer2.balanceAfter = 10500.0;
    transfer2.description = "收到来自张三（3456）的转账";
    transfer2.targetCardNumber = testCard1;
    m_transactions.append(transfer2);
    
    // 2. 查询余额
    Transaction inquiry1;
    inquiry1.cardNumber = testCard2;
    inquiry1.timestamp = now.addSecs(-12 * 3600); // 12小时前
    inquiry1.type = TransactionType::BalanceInquiry;
    inquiry1.amount = 0.0;
    inquiry1.balanceAfter = 10500.0;
    inquiry1.description = "余额查询";
    m_transactions.append(inquiry1);
    
    qDebug() << "已初始化" << m_transactions.size() << "条测试交易记录";
}

// 新增的业务逻辑方法，用于创建交易记录
Transaction TransactionModel::createTransaction(const QString &cardNumber, TransactionType type, 
                                            double amount, double balanceAfter, 
                                            const QString &description, const QString &targetCard)
{
    Transaction transaction;
    transaction.cardNumber = cardNumber;
    transaction.timestamp = QDateTime::currentDateTime();
    transaction.type = type;
    transaction.amount = amount;
    transaction.balanceAfter = balanceAfter;
    transaction.description = description;
    transaction.targetCardNumber = targetCard;
    
    return transaction;
}

// 创建交易记录并添加到模型中
void TransactionModel::recordTransaction(const QString &cardNumber, TransactionType type, 
                                      double amount, double balanceAfter, 
                                      const QString &description, const QString &targetCard)
{
    Transaction transaction = createTransaction(cardNumber, type, amount, balanceAfter, description, targetCard);
    addTransaction(transaction);
}

// 创建转账目标账户的交易记录（存款类型）
void TransactionModel::recordTransferReceipt(const QString &fromCardNumber, const QString &fromCardHolderName, 
                                          const QString &toCardNumber, double amount, double balanceAfter)
{
    QString description = QString("收到来自%1（%2）的转账").arg(fromCardHolderName).arg(fromCardNumber.right(4));
    
    Transaction receiverTransaction = createTransaction(
        toCardNumber,
        TransactionType::Deposit,
        amount,
        balanceAfter,
        description,
        fromCardNumber
    );
    
    addTransaction(receiverTransaction);
}