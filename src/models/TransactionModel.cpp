// TransactionModel.cpp
/**
 * @file TransactionModel.cpp
 * @brief 交易数据模型实现文件
 *
 * 实现了 TransactionModel 类中定义的交易数据管理和格式化方法。
 * 负责与持久化存储交互（模拟 JSON 文件存储）并处理交易记录。
 */
#include "TransactionModel.h"
#include <algorithm> // 用于 std::sort 和 std::remove_if
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QFile>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TransactionModel::TransactionModel(QObject *parent)
    : QObject(parent)
{
    // 设置数据存储路径到应用程序的本地数据目录。
    m_dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(m_dataPath);
    // 如果目录不存在，则创建它。
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    qDebug() << "交易记录存储路径:" << m_dataPath;

    // 尝试从文件加载交易记录。
    // 如果加载失败，则初始化测试交易并保存到文件。
    if (!loadTransactions()) {
        qDebug() << "无法加载交易记录，初始化测试交易";
        initializeTestTransactions();
        saveTransactions(); // 保存初始化的测试交易
    }
}

/**
 * @brief 析构函数
 *
 * 在对象销毁时保存交易记录。
 */
TransactionModel::~TransactionModel()
{
    // 在析构函数中保存交易记录
    saveTransactions();
}

/**
 * @brief 添加交易记录到内存列表并保存
 * @param transaction 要添加的 Transaction 对象
 */
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

/**
 * @brief 获取指定卡号的所有交易记录
 * @param cardNumber 卡号
 * @return 包含该卡号所有交易记录的 QVector
 */
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

/**
 * @brief 获取指定卡号的最近交易记录
 * @param cardNumber 卡号
 * @param count 要获取的记录数量
 * @return 包含指定数量最近交易记录的 QVector
 */
QVector<Transaction> TransactionModel::getRecentTransactions(const QString &cardNumber, int count) const
{
    QVector<Transaction> transactions = getTransactionsForCard(cardNumber);

    // 按时间戳排序（最新的在前）
    std::sort(transactions.begin(), transactions.end(),
              [](const Transaction &a, const Transaction &b) {
                  return a.timestamp > b.timestamp;
              });

    // 限制为请求的数量
    if (transactions.size() > count) {
        transactions.resize(count);
    }

    qDebug() << "返回" << transactions.size() << "条最近交易记录，请求数量为" << count;
    return transactions;
}

/**
 * @brief 清除指定卡号的所有交易记录
 *
 * 例如在删除账户时使用。
 *
 * @param cardNumber 卡号
 */
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

/**
 * @brief 保存交易记录到文件
 * @param filename 文件名 (默认为 transactions.json)
 * @return 如果成功保存返回 true，否则返回 false
 */
bool TransactionModel::saveTransactions(const QString &filename)
{
    QString filePath = m_dataPath + "/" + filename;
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "无法打开文件保存交易记录:" << filePath << ", 错误:" << file.errorString();
        return false;
    }

    QJsonArray transactionsArray;

    // 将所有交易记录转换为 JSON 数组
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

/**
 * @brief 从文件加载交易记录
 * @param filename 文件名 (默认为 transactions.json)
 * @return 如果成功加载返回 true，否则返回 false
 */
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

    // 从 JSON 数组加载交易记录
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

/**
 * @brief 初始化测试交易数据
 *
 * 如果数据文件不存在，则创建一些预设的测试交易记录。
 */
void TransactionModel::initializeTestTransactions()
{
    // 测试卡号，与 AccountModel 中的测试账户一致
    QString testCard1 = "1234567890123456"; // 张三
    QString testCard2 = "2345678901234567"; // 李四

    // 当前时间
    QDateTime now = QDateTime::currentDateTime();

    // 为张三添加一些测试交易
    // 1. 存款交易
    Transaction deposit1;
    deposit1.cardNumber = testCard1;
    deposit1.timestamp = now.addSecs(-5 * 24 * 3600); // 5 天前
    deposit1.type = TransactionType::Deposit;
    deposit1.amount = 1000.0;
    deposit1.balanceAfter = 6000.0;
    deposit1.description = "ATM 存款";
    m_transactions.append(deposit1);

    // 2. 取款交易
    Transaction withdraw1;
    withdraw1.cardNumber = testCard1;
    withdraw1.timestamp = now.addSecs(-3 * 24 * 3600); // 3 天前
    withdraw1.type = TransactionType::Withdrawal;
    withdraw1.amount = 500.0;
    withdraw1.balanceAfter = 5500.0;
    withdraw1.description = "ATM 取款";
    m_transactions.append(withdraw1);

    // 3. 转账交易 (转出方)
    Transaction transfer1;
    transfer1.cardNumber = testCard1;
    transfer1.timestamp = now.addSecs(-1 * 24 * 3600); // 1 天前
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
    transfer2.timestamp = now.addSecs(-1 * 24 * 3600); // 1 天前
    transfer2.type = TransactionType::Deposit; // 收到的转账对收款方来说是存款
    transfer2.amount = 500.0;
    transfer2.balanceAfter = 10500.0;
    transfer2.description = "收到来自张三（3456）的转账";
    transfer2.targetCardNumber = testCard1; // 记录发送方卡号
    m_transactions.append(transfer2);

    // 2. 查询余额
    Transaction inquiry1;
    inquiry1.cardNumber = testCard2;
    inquiry1.timestamp = now.addSecs(-12 * 3600); // 12 小时前
    inquiry1.type = TransactionType::BalanceInquiry;
    inquiry1.amount = 0.0; // 余额查询金额为 0
    inquiry1.balanceAfter = 10500.0;
    inquiry1.description = "余额查询";
    m_transactions.append(inquiry1);

    qDebug() << "已初始化" << m_transactions.size() << "条测试交易记录";
}

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
void TransactionModel::recordTransaction(const QString &cardNumber, TransactionType type,
                                      double amount, double balanceAfter,
                                      const QString &description, const QString &targetCard)
{
    Transaction transaction = createTransaction(cardNumber, type, amount, balanceAfter, description, targetCard);
    addTransaction(transaction);
}

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
void TransactionModel::recordTransferReceipt(const QString &fromCardNumber, const QString &fromCardHolderName,
                                          const QString &toCardNumber, double amount, double balanceAfter)
{
    QString description = QString("收到来自%1（%2）的转账").arg(fromCardHolderName).arg(fromCardNumber.right(4));

    Transaction receiverTransaction = createTransaction(
        toCardNumber,
        TransactionType::Deposit, // 收到的转账是存款
        amount,
        balanceAfter,
        description,
        fromCardNumber // 记录发送方卡号
    );

    addTransaction(receiverTransaction);
}

/**
 * @brief 格式化金额为货币字符串
 * @param amount 金额
 * @return 格式化后的金额字符串
 */
QString TransactionModel::formatAmount(double amount) const
{
    QLocale locale = QLocale::system();
    // 格式化为货币，保留两位小数
    return locale.toString(amount, 'f', 2);
}

/**
 * @brief 格式化日期时间为字符串
 * @param dateTime 日期时间对象
 * @return 格式化后的日期时间字符串 (yyyy-MM-dd hh:mm:ss)
 */
QString TransactionModel::formatDate(const QDateTime &dateTime) const
{
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

/**
 * @brief 获取交易类型的显示名称
 * @param type 交易类型枚举的整型值
 * @return 交易类型的中文名称
 */
QString TransactionModel::getTransactionTypeName(int type) const
{
    switch (static_cast<TransactionType>(type)) {
        case TransactionType::Deposit:
            return "存款";
        case TransactionType::Withdrawal:
            return "取款";
        case TransactionType::BalanceInquiry:
            return "余额查询";
        case TransactionType::Transfer:
            return "转账";
        case TransactionType::Other:
        default:
            return "其他";
    }
}