// PrinterModel.h
/**
 * @file PrinterModel.h
 * @brief 打印模型头文件
 *
 * 定义了 PrinterModel 类，负责生成打印内容（例如回单的 HTML）和执行实际的打印操作。
 * 它不直接与 UI 交互，而是由 ViewModel 调用。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QPrinter>
#include <QDateTime>
#include <QPrinterInfo>
#include <QTextDocument>
#include <QPageLayout> // 包含 QPageLayout 头文件

/**
 * @brief 打印模型类
 *
 * 负责生成打印内容和实际的打印功能。
 * 处理与打印机相关的底层操作。
 */
class PrinterModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit PrinterModel(QObject *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~PrinterModel();

    /**
     * @brief 打印回单
     *
     * 将生成的 HTML 内容发送到打印机。
     *
     * @param htmlContent 回单的 HTML 内容
     * @return 如果打印成功返回 true，否则返回 false
     */
    bool printReceipt(const QString &htmlContent);

    /**
     * @brief 生成回单的 HTML 内容
     *
     * 根据交易详情生成用于打印的 HTML 格式回单内容。
     *
     * @param bankName 银行名称
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param transactionType 交易类型 (例如："存款", "取款", "转账")
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param targetCardNumber 目标卡号（转账时使用，默认为空）
     * @param targetCardHolder 目标持卡人姓名（转账时使用，默认为空）
     * @param transactionDate 交易日期时间（默认为当前时间）
     * @param transactionId 交易编号（默认为空，将自动生成）
     * @return 生成的 HTML 内容字符串
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
    /**
     * @brief 初始化打印机设置
     *
     * 配置 QPrinter 对象，如纸张大小、方向和默认打印机。
     */
    void initializePrinter();

    //!< QPrinter 对象，用于进行打印操作
    QPrinter *m_printer;
};