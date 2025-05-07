#include "TransactionViewModel.h"
#include <QLocale>
#include <QDebug>

TransactionViewModel::TransactionViewModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_recentTransactionCount(10)
    , m_transactionModel(nullptr)
{
}

int TransactionViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    
    return m_transactions.size();
}

QVariant TransactionViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_transactions.size())
        return QVariant();
    
    const Transaction &transaction = m_transactions.at(index.row());
    
    switch (role) {
        case TypeRole:
            return static_cast<int>(transaction.type);
        case AmountRole:
            return transaction.amount;
        case BalanceAfterRole:
            return transaction.balanceAfter;
        case TimestampRole:
            return transaction.timestamp;
        case DescriptionRole:
            return transaction.description;
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> TransactionViewModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TypeRole] = "type";
    roles[AmountRole] = "amount";
    roles[BalanceAfterRole] = "balanceAfter";
    roles[TimestampRole] = "timestamp";
    roles[DescriptionRole] = "description";
    return roles;
}

QString TransactionViewModel::cardNumber() const
{
    return m_cardNumber;
}

void TransactionViewModel::setCardNumber(const QString &cardNumber)
{
    if (m_cardNumber != cardNumber) {
        m_cardNumber = cardNumber;
        emit cardNumberChanged();
        refreshTransactions();
    }
}

int TransactionViewModel::recentTransactionCount() const
{
    return m_recentTransactionCount;
}

void TransactionViewModel::setRecentTransactionCount(int count)
{
    if (m_recentTransactionCount != count && count > 0) {
        m_recentTransactionCount = count;
        emit recentTransactionCountChanged();
        refreshTransactions();
    }
}

void TransactionViewModel::setTransactionModel(TransactionModel *model)
{
    m_transactionModel = model;
    refreshTransactions();
}

void TransactionViewModel::refreshTransactions()
{
    if (!m_transactionModel || m_cardNumber.isEmpty()) {
        beginResetModel();
        m_transactions.clear();
        endResetModel();
        return;
    }
    
    beginResetModel();
    m_transactions = m_transactionModel->getRecentTransactions(m_cardNumber, m_recentTransactionCount);
    endResetModel();
    
    qDebug() << "刷新交易记录: 卡号=" << m_cardNumber << ", 找到记录数=" << m_transactions.size();
}

QString TransactionViewModel::formatAmount(double amount) const
{
    // 调用Model层的格式化方法
    if (m_transactionModel) {
        return m_transactionModel->formatAmount(amount);
    }
    
    // 如果Model不可用，提供备用实现
    QLocale locale = QLocale::system();
    return locale.toString(amount, 'f', 2);
}

QString TransactionViewModel::formatDate(const QDateTime &dateTime) const
{
    // 调用Model层的格式化方法
    if (m_transactionModel) {
        return m_transactionModel->formatDate(dateTime);
    }
    
    // 如果Model不可用，提供备用实现
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

QString TransactionViewModel::getTransactionTypeName(int type) const
{
    // 调用Model层的格式化方法
    if (m_transactionModel) {
        return m_transactionModel->getTransactionTypeName(type);
    }
    
    // 如果Model不可用，提供备用实现
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