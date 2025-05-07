#pragma once

#include <QObject>
#include <QAbstractListModel>
#include "../models/TransactionModel.h"

class TransactionViewModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    Q_PROPERTY(int recentTransactionCount READ recentTransactionCount WRITE setRecentTransactionCount NOTIFY recentTransactionCountChanged)

public:
    // Model roles
    enum TransactionRoles {
        TypeRole = Qt::UserRole + 1,
        AmountRole,
        BalanceAfterRole,
        TimestampRole,
        DescriptionRole
    };

    explicit TransactionViewModel(QObject *parent = nullptr);

    // QAbstractListModel overrides
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Property getters/setters
    QString cardNumber() const;
    Q_INVOKABLE void setCardNumber(const QString &cardNumber);
    int recentTransactionCount() const;
    Q_INVOKABLE void setRecentTransactionCount(int count);
    
    // 添加直接更新卡号并刷新交易的方法（供QML使用）
    Q_INVOKABLE void updateCardNumber(const QString &cardNumber);

    // Set the transaction model reference
    void setTransactionModel(TransactionModel *model);
    
    // Refresh transactions
    Q_INVOKABLE void refreshTransactions();
    
    // Helper methods for QML
    Q_INVOKABLE QString formatAmount(double amount) const;
    Q_INVOKABLE QString formatDate(const QDateTime &dateTime) const;
    Q_INVOKABLE QString getTransactionTypeName(int type) const;

signals:
    void cardNumberChanged();
    void recentTransactionCountChanged();

private:
    QString m_cardNumber;
    int m_recentTransactionCount;
    TransactionModel *m_transactionModel;
    QVector<Transaction> m_transactions;
};