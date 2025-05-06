#include "PrinterViewModel.h"
#include <QDebug>

PrinterViewModel::PrinterViewModel(QObject *parent)
    : QObject(parent)
{
}

PrinterViewModel::~PrinterViewModel()
{
}

bool PrinterViewModel::printDepositReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    double amount,
    double balanceAfter,
    const QString &transactionId)
{
    qDebug() << "打印存款回单" 
             << "卡号:" << cardNumber 
             << "持卡人:" << holderName 
             << "金额:" << amount 
             << "余额:" << balanceAfter;
             
    QString htmlContent = m_printerModel.generateReceiptHtml(
        bankName,
        cardNumber,
        holderName,
        "存款",
        amount,
        balanceAfter,
        QString(),
        QString(),
        QDateTime::currentDateTime(),
        transactionId
    );
    
    return m_printerModel.printReceipt(htmlContent, true);
}

bool PrinterViewModel::printWithdrawalReceipt(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    double amount,
    double balanceAfter,
    const QString &transactionId)
{
    qDebug() << "打印取款回单" 
             << "卡号:" << cardNumber 
             << "持卡人:" << holderName 
             << "金额:" << amount 
             << "余额:" << balanceAfter;
             
    QString htmlContent = m_printerModel.generateReceiptHtml(
        bankName,
        cardNumber,
        holderName,
        "取款",
        amount,
        balanceAfter,
        QString(),
        QString(),
        QDateTime::currentDateTime(),
        transactionId
    );
    
    return m_printerModel.printReceipt(htmlContent, true);
}

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
    qDebug() << "打印转账回单" 
             << "卡号:" << cardNumber 
             << "持卡人:" << holderName 
             << "金额:" << amount 
             << "余额:" << balanceAfter
             << "目标卡号:" << targetCardNumber
             << "目标持卡人:" << targetCardHolder;
             
    QString htmlContent = m_printerModel.generateReceiptHtml(
        bankName,
        cardNumber,
        holderName,
        "转账",
        amount,
        balanceAfter,
        targetCardNumber,
        targetCardHolder,
        QDateTime::currentDateTime(),
        transactionId
    );
    
    return m_printerModel.printReceipt(htmlContent, true);
} 