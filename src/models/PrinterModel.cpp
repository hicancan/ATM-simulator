#include "PrinterModel.h"
#include <QDebug>
#include <QGuiApplication>
#include <QUuid>
#include <QPrinterInfo>
#include <QPrintDialog>
#include <QWindow>

PrinterModel::PrinterModel(QObject *parent)
    : QObject(parent)
    , m_printer(nullptr)
{
    initializePrinter();
}

PrinterModel::~PrinterModel()
{
    if (m_printer) {
        delete m_printer;
        m_printer = nullptr;
    }
}

void PrinterModel::initializePrinter()
{
    if (!m_printer) {
        m_printer = new QPrinter(QPrinter::HighResolution);
        
        // 使用标准A4纸张大小，避免页面尺寸相关断言错误
        m_printer->setPageSize(QPageSize(QPageSize::A4));
        m_printer->setPageOrientation(QPageLayout::Portrait);
        
        // 获取默认打印机
        QString defaultPrinterName = QPrinterInfo::defaultPrinterName();
        if (!defaultPrinterName.isEmpty()) {
            m_printer->setPrinterName(defaultPrinterName);
            qDebug() << "设置默认打印机: " << defaultPrinterName;
        } else {
            qDebug() << "未找到默认打印机";
        }
    }
}

bool PrinterModel::printReceipt(const QString &htmlContent, bool showPrintDialog)
{
    if (!m_printer) {
        initializePrinter();
    }
    
    if (!m_printer) {
        qDebug() << "初始化打印机失败";
        return false;
    }
    
#ifdef Q_OS_WIN
    if (showPrintDialog) {
        return printWithWindowsDialog(htmlContent);
    }
#endif
    
    // 标准Qt打印方式
    QTextDocument document;
    document.setHtml(htmlContent);
    
    // 设置文档页面大小 - 避免使用可能导致max < min错误的方法
    QSizeF pageSize = m_printer->pageLayout().paintRect(QPageLayout::Point).size();
    if (pageSize.isValid() && pageSize.width() > 0 && pageSize.height() > 0) {
        document.setPageSize(pageSize);
    } else {
        // 使用安全的默认值
        document.setPageSize(QSizeF(595, 842)); // A4大小，单位为点
    }
    
    if (showPrintDialog) {
        QPrintDialog printDialog(m_printer, nullptr);
        if (printDialog.exec() != QDialog::Accepted) {
            return false;
        }
    }
    
    document.print(m_printer);
    return true;
}

QString PrinterModel::generateReceiptHtml(
    const QString &bankName,
    const QString &cardNumber,
    const QString &holderName,
    const QString &transactionType,
    double amount,
    double balanceAfter,
    const QString &targetCardNumber,
    const QString &targetCardHolder,
    const QDateTime &transactionDate,
    const QString &transactionId)
{
    QString uuid = transactionId.isEmpty() ? 
                  QUuid::createUuid().toString(QUuid::WithoutBraces).mid(0, 10) : 
                  transactionId;
    
    QString html = 
        "<html>"
        "<head>"
        "<style>"
        "body { font-family: Arial, sans-serif; font-size: 10pt; margin: 0; padding: 10px; }"
        "h1 { font-size: 12pt; text-align: center; margin-bottom: 10px; }"
        "table { width: 100%; border-collapse: collapse; }"
        "th, td { padding: 4px; text-align: left; }"
        "th { font-weight: bold; }"
        ".header, .footer { text-align: center; margin: 10px 0; }"
        ".divider { border-top: 1px solid black; margin: 10px 0; }"
        "</style>"
        "</head>"
        "<body>"
        "<div class='header'>"
        "<h1>" + bankName + "</h1>"
        "</div>"
        "<div class='divider'></div>"
        "<table>"
        "<tr><th>交易类型:</th><td>" + transactionType + "</td></tr>"
        "<tr><th>交易时间:</th><td>" + transactionDate.toString("yyyy-MM-dd hh:mm:ss") + "</td></tr>"
        "<tr><th>交易卡号:</th><td>尾号" + cardNumber.right(4) + "</td></tr>"
        "<tr><th>持卡人:</th><td>" + holderName + "</td></tr>"
        "<tr><th>交易金额:</th><td>￥" + QString::number(amount, 'f', 2) + "</td></tr>"
        "<tr><th>交易后余额:</th><td>￥" + QString::number(balanceAfter, 'f', 2) + "</td></tr>";
    
    // 如果是转账，添加目标账户信息
    if (transactionType == "转账" && !targetCardNumber.isEmpty()) {
        html += "<tr><th>收款卡号:</th><td>尾号" + targetCardNumber.right(4) + "</td></tr>";
        if (!targetCardHolder.isEmpty()) {
            html += "<tr><th>收款人:</th><td>" + targetCardHolder + "</td></tr>";
        }
    }
    
    html += "<tr><th>交易编号:</th><td>" + uuid + "</td></tr>"
            "</table>"
            "<div class='divider'></div>"
            "<div class='footer'>"
            "<p>此回单作为交易凭证，请妥善保管。<br>"
            "感谢您使用ATM模拟器银行服务！</p>"
            "</div>"
            "</body>"
            "</html>";
    
    return html;
}

#ifdef Q_OS_WIN
bool PrinterModel::printWithWindowsDialog(const QString &htmlContent)
{
    PRINTDLGEX pdex;
    if (!setupWindowsPrintDialog(pdex)) {
        return false;
    }
    
    // 如果用户选择了打印机
    if (pdex.dwResultAction == PD_RESULT_PRINT) {
        HDC hdc = pdex.hDC;
        if (hdc) {
            // 使用Qt的方式打印到选定的打印机
            QTextDocument document;
            document.setHtml(htmlContent);
            
            // 使用安全的默认页面大小
            document.setPageSize(QSizeF(595, 842)); // A4大小，单位为点
            
            // 获取打印机名称
            wchar_t printerName[256];
            if (GetPrinterNameFromHDC(hdc, printerName, 256)) {
                QString qtPrinterName = QString::fromWCharArray(printerName);
                
                // 设置Qt打印机
                m_printer->setPrinterName(qtPrinterName);
            }
            
            // 打印文档
            document.print(m_printer);
            
            // 释放DC
            DeleteDC(hdc);
            return true;
        }
    }
    
    // 释放资源
    if (pdex.hDevMode) GlobalFree(pdex.hDevMode);
    if (pdex.hDevNames) GlobalFree(pdex.hDevNames);
    
    return false;
}

bool PrinterModel::setupWindowsPrintDialog(PRINTDLGEX &pdex)
{
    // 初始化打印对话框结构
    ZeroMemory(&pdex, sizeof(PRINTDLGEX));
    pdex.lStructSize = sizeof(PRINTDLGEX);
    
    QWindow *window = QGuiApplication::focusWindow();
    if (window) {
        pdex.hwndOwner = (HWND)window->winId();
    } else {
        pdex.hwndOwner = NULL;
    }
    
    pdex.Flags = PD_RETURNDC | PD_USEDEVMODECOPIES | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_NOPAGENUMS;
    pdex.nStartPage = START_PAGE_GENERAL;
    pdex.nCopies = 1;
    
    // 显示打印对话框
    HRESULT hr = PrintDlgEx(&pdex);
    if (hr != S_OK) {
        qDebug() << "打印对话框显示失败: " << hr;
        return false;
    }
    
    return true;
}

// 辅助函数：从HDC获取打印机名称
bool PrinterModel::GetPrinterNameFromHDC(HDC hdc, LPWSTR printerName, DWORD bufferSize)
{
    if (!hdc) {
        wcscpy_s(printerName, bufferSize, L"DefaultPrinter");
        return false;
    }
    
    // 这里简化处理，使用默认打印机名称
    wcscpy_s(printerName, bufferSize, L"DefaultPrinter");
    
    // 获取打印机名称 - 这部分函数在某些Windows版本可能不可用，使用安全方式
    DWORD size = 0;
    if (::GetPrinter(hdc, 1, NULL, 0, &size) || GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        if (size > 0) {
            BYTE* buffer = new BYTE[size];
            if (::GetPrinter(hdc, 1, buffer, size, &size)) {
                PRINTER_INFO_1* pi1 = (PRINTER_INFO_1*)buffer;
                if (pi1 && pi1->pName) {
                    wcscpy_s(printerName, bufferSize, pi1->pName);
                    delete[] buffer;
                    return true;
                }
            }
            delete[] buffer;
        }
    }
    
    return true;
}
#endif 