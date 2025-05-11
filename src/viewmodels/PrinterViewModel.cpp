// PrinterViewModel.cpp
/**
 * @file PrinterViewModel.cpp
 * @brief 打印视图模型实现文件
 *
 * 实现了 PrinterViewModel 类中定义的可调用方法。
 * 作为视图模型层，它接收来自 UI (QML) 的请求，
 * 调用 PrinterModel 的相应方法来处理打印逻辑。
 */
#include "PrinterViewModel.h"
#include <QDebug> // 包含调试输出头文件

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PrinterViewModel::PrinterViewModel(QObject *parent)
    : QObject(parent)
{
    // 构造函数，无特殊初始化逻辑
}

/**
 * @brief 打印存款回单
 *
 * 调用 PrinterModel 生成存款回单的 HTML 内容，并触发打印。
 *
 * @param bankName 银行名称
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param amount 存款金额
 * @param balanceAfter 存款后余额
 * @param transactionId 交易编号
 * @return 如果打印成功返回 true，否则返回 false
 */
bool PrinterViewModel::printDepositReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    double amount,
    double balanceAfter,
    const QString &transactionId)
{
    // 调用通用打印方法
    return printReceipt(
        bankName,
        cardNumber,
        holderName,
        "存款", // 交易类型描述
        amount,
        balanceAfter,
        QString(), // 存款无需目标卡号
        QString(), // 存款无需目标持卡人
        transactionId
    );
}

/**
 * @brief 打印取款回单
 *
 * 调用 PrinterModel 生成取款回单的 HTML 内容，并触发打印。
 *
 * @param bankName 银行名称
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param amount 取款金额
 * @param balanceAfter 取款后余额
 * @param transactionId 交易编号
 * @return 如果打印成功返回 true，否则返回 false
 */
bool PrinterViewModel::printWithdrawalReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    double amount,
    double balanceAfter,
    const QString &transactionId)
{
    // 调用通用打印方法
    return printReceipt(
        bankName,
        cardNumber,
        holderName,
        "取款", // 交易类型描述
        amount,
        balanceAfter,
        QString(), // 取款无需目标卡号
        QString(), // 取款无需目标持卡人
        transactionId
    );
}

/**
 * @brief 打印转账回单
 *
 * 调用 PrinterModel 生成转账回单的 HTML 内容，并触发打印。
 *
 * @param bankName 银行名称
 * @param cardNumber 转出卡号
 * @param holderName 转出持卡人姓名
 * @param amount 转账金额
 * @param balanceAfter 转账后余额
 * @param targetCardNumber 转入卡号
 * @param targetCardHolder 转入持卡人姓名
 * @param transactionId 交易编号
 * @return 如果打印成功返回 true，否则返回 false
 */
bool PrinterViewModel::printTransferReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    double amount,
    double balanceAfter,
    const QString &targetCardNumber,
    const QString &targetCardHolder,
    const QString &transactionId)
{
    // 调用通用打印方法
    return printReceipt(
        bankName,
        cardNumber,
        holderName,
        "转账", // 交易类型描述
        amount,
        balanceAfter,
        targetCardNumber,
        targetCardHolder,
        transactionId
    );
}

/**
 * @brief 通用打印回单方法
 *
 * 调用 PrinterModel 生成回单的 HTML 内容，并触发打印。此方法统一处理所有类型的回单打印。
 *
 * @param bankName 银行名称
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param transactionType 交易类型描述
 * @param amount 交易金额
 * @param balanceAfter 交易后余额
 * @param targetCardNumber 目标卡号（转账时使用）
 * @param targetCardHolder 目标持卡人姓名（转账时使用）
 * @param transactionId 交易编号
 * @return 如果打印成功返回 true，否则返回 false
 */
bool PrinterViewModel::printReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    const QString &transactionType,
    double amount,
    double balanceAfter,
    const QString &targetCardNumber,
    const QString &targetCardHolder,
    const QString &transactionId)
{
    // 记录打印请求日志
    qDebug() << "打印" << transactionType << "回单"
             << "卡号:" << cardNumber
             << "持卡人:" << holderName
             << "金额:" << amount
             << "余额:" << balanceAfter;
    
    if (!targetCardNumber.isEmpty()) {
        qDebug() << "目标卡号:" << targetCardNumber
                 << "目标持卡人:" << targetCardHolder;
    }

    // 调用 PrinterModel 生成回单的 HTML 内容
    QString htmlContent = m_printerModel.generateReceiptHtml(
        bankName,
        cardNumber,
        holderName,
        transactionType,
        amount,
        balanceAfter,
        targetCardNumber,
        targetCardHolder,
        QDateTime::currentDateTime(), // 使用当前时间作为交易时间
        transactionId // 传递交易 ID
    );

    // 调用 PrinterModel 打印生成的 HTML 内容
    return m_printerModel.printReceipt(htmlContent);
}