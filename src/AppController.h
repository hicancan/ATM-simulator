#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include "viewmodels/AccountViewModel.h"
#include "viewmodels/TransactionViewModel.h"
#include "viewmodels/PrinterViewModel.h"

// 声明指针类型为不透明指针，以便元对象系统可以处理
Q_DECLARE_OPAQUE_POINTER(AccountViewModel*)
Q_DECLARE_OPAQUE_POINTER(TransactionViewModel*)
Q_DECLARE_OPAQUE_POINTER(PrinterViewModel*)

class AppController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(AccountViewModel* accountViewModel READ accountViewModel CONSTANT)
    Q_PROPERTY(TransactionViewModel* transactionViewModel READ transactionViewModel CONSTANT)
    Q_PROPERTY(PrinterViewModel* printerViewModel READ printerViewModel CONSTANT)
    Q_PROPERTY(QString currentPage READ currentPage NOTIFY currentPageChanged)

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    void initialize(QQmlEngine* engine);

    // Property getters
    AccountViewModel* accountViewModel() const;
    TransactionViewModel* transactionViewModel() const;
    PrinterViewModel* printerViewModel() const;
    QString currentPage() const;

    // Invokable methods for QML
    Q_INVOKABLE void switchToPage(const QString &pageName);
    Q_INVOKABLE void logout();

signals:
    void currentPageChanged();

private:
    QString m_currentPage = "LoginPage";
    AccountViewModel* m_accountViewModel;
    TransactionViewModel* m_transactionViewModel;
    PrinterViewModel* m_printerViewModel;
}; 