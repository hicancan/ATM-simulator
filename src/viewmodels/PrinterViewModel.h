#pragma once

#include <QObject>
#include <QString>
#include <QDateTime>
#include "../models/PrinterModel.h"

class PrinterViewModel : public QObject
{
    Q_OBJECT

public:
    explicit PrinterViewModel(QObject *parent = nullptr);
    ~PrinterViewModel();

    // QML调用方法
    Q_INVOKABLE bool printDepositReceipt(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        double amount,
        double balanceAfter,
        const QString &transactionId = QString()
    );
    
    Q_INVOKABLE bool printWithdrawalReceipt(
        const QString &bankName,
        const QString &cardNumber,
        const QString &holderName,
        double amount,
        double balanceAfter,
        const QString &transactionId = QString()
    );
    
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
    PrinterModel m_printerModel;
}; 