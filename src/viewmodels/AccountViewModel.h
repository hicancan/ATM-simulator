#pragma once

#include <QObject>
#include <QString>
#include "../models/AccountModel.h"
#include "../models/TransactionModel.h"

class AccountViewModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    Q_PROPERTY(QString holderName READ holderName NOTIFY holderNameChanged)
    Q_PROPERTY(double balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(double withdrawLimit READ withdrawLimit NOTIFY withdrawLimitChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit AccountViewModel(QObject *parent = nullptr);

    // Property getters/setters
    QString cardNumber() const;
    Q_INVOKABLE void setCardNumber(const QString &cardNumber);
    QString holderName() const;
    double balance() const;
    double withdrawLimit() const;
    bool isLoggedIn() const;
    QString errorMessage() const;
    
    // Set the transaction model reference
    void setTransactionModel(TransactionModel *model);

    // Invokable methods for QML
    Q_INVOKABLE bool login(const QString &pin);
    Q_INVOKABLE bool loginWithCard(const QString &cardNumber, const QString &pin);
    Q_INVOKABLE bool withdraw(double amount);
    Q_INVOKABLE bool deposit(double amount);
    Q_INVOKABLE bool transfer(const QString &targetCard, double amount);
    Q_INVOKABLE bool validateTargetCard(const QString &targetCard);
    Q_INVOKABLE QString getTargetCardHolderName(const QString &targetCard);
    Q_INVOKABLE bool changePassword(const QString &currentPin, const QString &newPin, const QString &confirmPin);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void clearError();
    Q_INVOKABLE void setErrorMessage(const QString &message);

signals:
    void cardNumberChanged();
    void holderNameChanged();
    void balanceChanged();
    void withdrawLimitChanged();
    void isLoggedInChanged();
    void errorMessageChanged();
    void loggedOut();
    void transactionCompleted(bool success, const QString &message);
    void transactionRecorded();

private:
    AccountModel m_accountModel;
    TransactionModel *m_transactionModel;
    QString m_cardNumber;
    QString m_errorMessage;
    bool m_isLoggedIn;
    
    // Helper method to record transactions
    void recordTransaction(TransactionType type, double amount, double balanceAfter, const QString &description, const QString &targetCard = QString());
}; 