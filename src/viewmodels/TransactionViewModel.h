// TransactionViewModel.h
/**
 * @file TransactionViewModel.h
 * @brief 交易视图模型头文件
 *
 * 定义了 TransactionViewModel 类，作为 TransactionModel 和 UI (QML) 之间的中介。
 * 它提供交易记录列表模型接口，以便在 QML 中显示。
 */
#pragma once

#include <QObject>
#include <QAbstractListModel>
#include "../models/TransactionModel.h" // 包含 TransactionModel 头文件

/**
 * @brief 交易视图模型类
 *
 * 提供交易记录列表模型接口给 QML。
 * 从 TransactionModel 获取最近的交易记录，并暴露给 QML 的 ListView 等控件。
 */
class TransactionViewModel : public QAbstractListModel
{
    Q_OBJECT

    // Q_PROPERTY 宏将属性暴露给 QML
    Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    Q_PROPERTY(int recentTransactionCount READ recentTransactionCount WRITE setRecentTransactionCount NOTIFY recentTransactionCountChanged)

public:
    // ViewModel专用的交易类型枚举，与Model层枚举解耦
    enum TransactionViewType {
        Deposit = 0,        //!< 存款
        Withdrawal = 1,     //!< 取款
        BalanceInquiry = 2, //!< 余额查询
        Transfer = 3,       //!< 转账
        Other = 4           //!< 其他
    };
    Q_ENUM(TransactionViewType) // 将枚举暴露给 QML

    // 模型角色枚举，用于在 QML 中访问数据
    enum TransactionRoles {
        TypeRole = Qt::UserRole + 1, //!< 交易类型角色
        AmountRole,                  //!< 交易金额角色
        BalanceAfterRole,            //!< 交易后余额角色
        TimestampRole,               //!< 交易时间戳角色
        DescriptionRole,             //!< 交易描述角色
        // 如果需要，可以添加更多角色，例如 TargetCardNumberRole
    };
    Q_ENUM(TransactionRoles) // 将枚举暴露给 QML

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit TransactionViewModel(QObject *parent = nullptr);

    // --- QAbstractListModel 重写方法 ---
    /**
     * @brief 返回模型中的行数 (交易记录数量)
     * @param parent 父索引
     * @return 模型中的行数
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    /**
     * @brief 返回指定索引和角色的数据
     * @param index 模型索引
     * @param role 数据角色
     * @return 索引和角色对应的数据
     */
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    /**
     * @brief 返回角色名称映射
     *
     * 用于 QML 通过名称访问角色数据。
     *
     * @return 角色名称映射
     */
    QHash<int, QByteArray> roleNames() const override;

    // --- 属性获取和设置方法 (可调用供 QML 使用) ---
    QString cardNumber() const;
    /**
     * @brief 设置卡号并刷新交易记录列表
     * @param cardNumber 新的卡号
     */
    Q_INVOKABLE void setCardNumber(const QString &cardNumber);
    int recentTransactionCount() const;
    /**
     * @brief 设置要显示的最近交易记录数量并刷新列表
     * @param count 数量
     */
    Q_INVOKABLE void setRecentTransactionCount(int count);

    // --- 可调用方法 (供 QML 调用) ---
    /**
     * @brief 设置交易数据模型引用
     * @param model 交易数据模型指针
     */
    void setTransactionModel(TransactionModel *model);
    /**
     * @brief 刷新交易记录列表
     *
     * 从模型中重新获取最近的交易记录。
     */
    Q_INVOKABLE void refreshTransactions();

    // --- 辅助方法 (可调用供 QML 使用) ---
    /**
     * @brief 格式化金额为货币字符串
     *
     * 调用 TransactionModel 的格式化方法。
     *
     * @param amount 金额
     * @return 格式化后的金额字符串
     */
    Q_INVOKABLE QString formatAmount(double amount) const;
    /**
     * @brief 格式化日期时间为字符串
     *
     * 调用 TransactionModel 的格式化方法。
     *
     * @param dateTime 日期时间对象
     * @return 格式化后的日期时间字符串
     */
    Q_INVOKABLE QString formatDate(const QDateTime &dateTime) const;
    /**
     * @brief 获取交易类型的显示名称
     *
     * 调用 TransactionModel 的格式化方法。
     *
     * @param type 交易类型枚举的整型值
     * @return 交易类型的中文名称
     */
    Q_INVOKABLE QString getTransactionTypeName(int type) const;

    /**
     * @brief 将Model层交易类型转换为ViewModel层交易类型
     * @param modelType Model层的交易类型
     * @return 对应的ViewModel层交易类型
     */
    TransactionViewType convertTransactionType(TransactionType modelType) const;

signals:
    // 通知 QML 属性已改变的信号
    void cardNumberChanged();
    void recentTransactionCountChanged();

private:
    //!< 用于显示交易记录的卡号
    QString m_cardNumber;
    //!< 要显示的最近交易记录数量
    int m_recentTransactionCount;
    //!< TransactionModel 指针
    TransactionModel *m_transactionModel;
    //!< 最近交易记录的内存缓存
    QVector<Transaction> m_transactions;
};