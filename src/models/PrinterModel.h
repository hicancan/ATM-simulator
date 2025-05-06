#pragma once

#include <QObject>
#include <QString>
#include <QPrinter>
#include <QPrintDialog>
#include <QDialog>
#include <QPainter>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QTextFrame>
#include <QDateTime>
#include <QWindow>

#ifdef Q_OS_WIN
#include <Windows.h>
#include <commdlg.h>
#include <winspool.h>
#endif

class PrinterModel : public QObject
{
    Q_OBJECT

public:
    explicit PrinterModel(QObject *parent = nullptr);
    ~PrinterModel();

    // 打印回单方法
    bool printReceipt(const QString &htmlContent, bool showPrintDialog = true);
    
    // 生成HTML内容
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
    
    // Windows打印会话
#ifdef Q_OS_WIN
    bool printWithWindowsDialog(const QString &htmlContent);
    bool setupWindowsPrintDialog(PRINTDLGEX &pdex);
    bool GetPrinterNameFromHDC(HDC hdc, LPWSTR printerName, DWORD bufferSize);
#endif
}; 