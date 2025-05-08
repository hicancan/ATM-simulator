// PrinterViewModel.h
/**
 * @file PrinterViewModel.h
 * @brief 打印视图模型头文件
 *
 * 定义了 PrinterViewModel 类，作为 PrinterModel 和 UI (QML) 之间的中介。
 * 它暴露打印相关的功能（通过可调用方法）给 QML，用于触发各种回单的打印。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include "../models/PrinterModel.h" // 包含 PrinterModel 头文件

/**
 * @brief 打印视图模型类
 *
 * 提供打印功能到 UI (QML) 的接口。
 * 调用 PrinterModel 生成回单内容并触发打印。
 */
class PrinterViewModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PrinterViewModel(QObject *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~PrinterViewModel();

    // --- 可调用方法 (供 QML 调用) ---
    /**
     * @brief 打印存款回单
     *
     * 收集存款详情并调用 PrinterModel 生成并打印存款回单。
     *
     * @param bankName 银行名称
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param amount 存款金额
     * @param balanceAfter 存款后余额
     * @param transactionId 交易编号 (默认为空，由 Model 生成)
     * @return 如果打印成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool printDepositReceipt(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        double amount,
        double balanceAfter,
        const QString &transactionId = QString()
    );

    /**
     * @brief 打印取款回单
     *
     * 收集取款详情并调用 PrinterModel 生成并打印取款回单。
     *
     * @param bankName 银行名称
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param amount 取款金额
     * @param balanceAfter 取款后余额
     * @param transactionId 交易编号 (默认为空，由 Model 生成)
     * @return 如果打印成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool printWithdrawalReceipt(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        double amount,
        double balanceAfter,
        const QString &transactionId = QString()
    );

    /**
     * @brief 打印转账回单
     *
     * 收集转账详情并调用 PrinterModel 生成并打印转账回单。
     *
     * @param bankName 银行名称
     * @param cardNumber 转出卡号
     * @param holderName 转出持卡人姓名
     * @param amount 转账金额
     * @param balanceAfter 转账后余额
     * @param targetCardNumber 转入卡号
     * @param targetCardHolder 转入持卡人姓名
     * @param transactionId 交易编号 (默认为空，由 Model 生成)
     * @return 如果打印成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool printTransferReceipt(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        double amount,
        double balanceAfter,
        const QString &targetCardNumber,
        const QString &targetCardHolder,
        const QString &transactionId = QString()
    );

private:
    //!< PrinterModel 实例，用于生成回单内容和执行实际打印
    PrinterModel m_printerModel;
};