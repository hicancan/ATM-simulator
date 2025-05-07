#pragma once

#include <QObject>
#include <QString>
#include <QPrinter>
#include <QDateTime>
#include <QPrinterInfo>
#include <QTextDocument>

/**
 * @brief 打印模型类，负责生成打印内容和实际的打印功能
 */
class PrinterModel : public QObject
{
    Q_OBJECT

public:
    explicit PrinterModel(QObject *parent = nullptr);
    ~PrinterModel();

    /**
     * @brief 打印回单
     * @param htmlContent 回单的HTML内容
     * @param showPrintDialog 是否显示打印对话框
     * @return 是否打印成功
     */
    bool printReceipt(const QString &htmlContent, bool showPrintDialog = true);
    
    /**
     * @brief 生成回单的HTML内容
     * @param bankName 银行名称
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param transactionType 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param targetCardNumber 目标卡号（转账时使用）
     * @param targetCardHolder 目标持卡人（转账时使用）
     * @param transactionDate 交易日期时间
     * @param transactionId 交易编号
     * @return 生成的HTML内容
     */
    QString generateReceiptHtml(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        const QString &transactionType,
        double amount,
        double balanceAfter,
        const QString &targetCardNumber = QString(),
        const QString &targetCardHolder = QString(),
        const QDateTime &transactionDate = QDateTime::currentDateTime(),
        const QString &transactionId = QString()
    );
    
private:
    // 初始化打印机
    void initializePrinter();
    
    // 打印机对象
    QPrinter *m_printer;
};