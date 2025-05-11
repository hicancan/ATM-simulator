// TransactionViewModel.cpp
/**
 * @file TransactionViewModel.cpp
 * @brief 交易视图模型实现文件
 *
 * 实现了 TransactionViewModel 类中定义的列表模型接口和可调用方法。
 * 从 TransactionModel 获取交易数据并提供给 QML。
 */
#include "TransactionViewModel.h"
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
TransactionViewModel::TransactionViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_recentTransactionCount(10) //!< 默认显示最近 10 条交易记录
    , m_transactionModel(nullptr)  //!< 初始化交易模型指针为空
{
    // 构造函数初始化成员变量，无复杂逻辑。
}

/**
 * @brief 返回模型中的行数 (交易记录数量)
 * @param parent 父索引
 * @return 模型中的行数
 */
int TransactionViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0; // 平坦列表，没有子项

    return m_transactions.size(); // 返回缓存的交易记录数量
}

/**
 * @brief 返回指定索引和角色的数据
 * @param index 模型索引
 * @param role 数据角色
 * @return 索引和角色对应的数据
 */
QVariant TransactionViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_transactions.size())
        return QVariant(); // 无效索引或越界

    const Transaction &transaction = m_transactions.at(index.row()); // 获取交易数据

    // 根据请求的角色返回数据
    switch (role) {
        case TypeRole:
            // 使用转换方法，返回ViewModel层的枚举值
            return static_cast<int>(convertTransactionType(transaction.type));
        case AmountRole:
            return transaction.amount;
        case BalanceAfterRole:
            return transaction.balanceAfter;
        case TimestampRole:
            return transaction.timestamp;
        case DescriptionRole:
            return transaction.description;
        // 添加其他角色的处理
        default:
            return QVariant(); // 未处理的角色
    }
}

/**
 * @brief 返回角色名称映射
 *
 * 用于 QML 通过名称访问角色数据。
 *
 * @return 角色名称映射
 */
QHash<int, QByteArray> TransactionViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[AmountRole] = "amount";
    roles[BalanceAfterRole] = "balanceAfter";
    roles[TimestampRole] = "timestamp";
    roles[DescriptionRole] = "description";
    // 添加其他角色的名称
    return roles;
}

// --- 属性获取和设置方法 ---

/**
 * @brief 获取当前卡号
 * @return 当前卡号
 */
QString TransactionViewModel::cardNumber() const
{
    return m_cardNumber;
}

/**
 * @brief 设置卡号并刷新交易记录列表
 * @param cardNumber 新的卡号
 */
void TransactionViewModel::setCardNumber(const QString &cardNumber)
{
    if (m_cardNumber != cardNumber) {
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        
        // 添加刷新交易记录的功能
        refreshTransactions();
    }
}

/**
 * @brief 获取要显示的最近交易记录数量
 * @return 数量
 */
int TransactionViewModel::recentTransactionCount() const
{
    return m_recentTransactionCount;
}

/**
 * @brief 设置要显示的最近交易记录数量并刷新列表
 * @param count 数量
 */
void TransactionViewModel::setRecentTransactionCount(int count)
{
    // 确保数量是正数且与当前数量不同
    if (m_recentTransactionCount != count && count > 0) {
        m_recentTransactionCount = count;
        emit recentTransactionCountChanged(); // 通知 QML
        refreshTransactions(); // 刷新以显示新数量的记录
    }
}

/**
 * @brief 设置交易数据模型引用
 * @param model 交易数据模型指针
 */
void TransactionViewModel::setTransactionModel(TransactionModel *model)
{
    m_transactionModel = model;
    // 设置模型后立即刷新交易记录
    refreshTransactions();
}

/**
 * @brief 刷新交易记录列表
 *
 * 从模型中重新获取最近的交易记录。
 */
void TransactionViewModel::refreshTransactions()
{
    beginResetModel(); // 在数据改变前通知 QML
    
    // 仅当 TransactionModel 已设置且卡号可用时才获取交易
    if (m_transactionModel && !m_cardNumber.isEmpty()) {
        // 从模型中获取最近的交易记录
        m_transactions = m_transactionModel->getRecentTransactions(m_cardNumber, m_recentTransactionCount);
        qDebug() << "刷新交易记录: 卡号=" << m_cardNumber << ", 找到记录数=" << m_transactions.size();
    } else {
        // 清空当前列表
        m_transactions.clear();
    }
    
    endResetModel(); // 在数据改变后通知 QML
}

// --- 辅助方法 (可调用供 QML 使用) ---

/**
 * @brief 格式化金额为货币字符串
 * @param amount 金额
 * @return 格式化后的金额字符串
 */
QString TransactionViewModel::formatAmount(double amount) const
{
    // 直接调用TransactionModel的方法
    if (m_transactionModel) {
        return m_transactionModel->formatAmount(amount);
    }
    qWarning() << "formatAmount: TransactionModel未设置";
    return QString::number(amount, 'f', 2);
}

/**
 * @brief 格式化日期时间为字符串
 * @param dateTime 日期时间对象
 * @return 格式化后的日期时间字符串
 */
QString TransactionViewModel::formatDate(const QDateTime &dateTime) const
{
    // 直接调用TransactionModel的方法
    if (m_transactionModel) {
        return m_transactionModel->formatDate(dateTime);
    }
    qWarning() << "formatDate: TransactionModel未设置";
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

/**
 * @brief 获取交易类型的显示名称
 * @param type 交易类型枚举的整型值
 * @return 交易类型的中文名称
 */
QString TransactionViewModel::getTransactionTypeName(int type) const
{
    // 直接基于ViewModel专用枚举返回名称，不再依赖Model层枚举
    switch (static_cast<TransactionViewType>(type)) {
        case TransactionViewType::Deposit:
            return "存款";
        case TransactionViewType::Withdrawal:
            return "取款";
        case TransactionViewType::BalanceInquiry:
            return "余额查询";
        case TransactionViewType::Transfer:
            return "转账";
        case TransactionViewType::Other:
        default:
            return "其他";
    }
}

/**
 * @brief 将Model层交易类型转换为ViewModel层交易类型
 * @param modelType Model层的交易类型
 * @return 对应的ViewModel层交易类型
 */
TransactionViewModel::TransactionViewType TransactionViewModel::convertTransactionType(TransactionType modelType) const
{
    switch (modelType) {
        case TransactionType::Deposit:
            return TransactionViewType::Deposit;
        case TransactionType::Withdrawal:
            return TransactionViewType::Withdrawal;
        case TransactionType::BalanceInquiry:
            return TransactionViewType::BalanceInquiry;
        case TransactionType::Transfer:
            return TransactionViewType::Transfer;
        case TransactionType::Other:
        default:
            return TransactionViewType::Other;
    }
}