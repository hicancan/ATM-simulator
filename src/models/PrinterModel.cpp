#include "PrinterModel.h"
#include <QDebug>
#include <QGuiApplication>
#include <QUuid>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPainter>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTextStream>
#include <QFile>

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
        
        // 使用标准A4纸张大小
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
    try {
        // 首先将HTML内容保存到临时文件，然后将其转换为PDF
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (!QDir(documentsPath).exists()) {
            QDir().mkpath(documentsPath);
        }
        
        // 创建唯一的文件名
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString pdfPath = documentsPath + "/ATM_Receipt_" + timestamp + ".pdf";
        QString htmlPath = documentsPath + "/ATM_Receipt_" + timestamp + ".html";
        
        // 保存HTML到临时文件
        QFile htmlFile(htmlPath);
        if (!htmlFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "无法创建HTML文件:" << htmlPath;
            return false;
        }
        
        QTextStream stream(&htmlFile);
        stream.setEncoding(QStringConverter::Utf8);
        stream << htmlContent;
        htmlFile.close();
        
        qDebug() << "已保存HTML文件:" << htmlPath;
        qDebug() << "HTML内容长度:" << htmlContent.length() << "字符";
        
        // 使用更直接的方式创建PDF
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(pdfPath);
        printer.setPageSize(QPageSize(QPageSize::A4));
        printer.setPageOrientation(QPageLayout::Portrait);
        
        // 关键修复: 设置更小的页面边距，充分利用整个页面
        printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
        
        // 创建文档并设置合适的样式和内容
        QTextDocument doc;
        doc.setDocumentMargin(0); // 减少文档内边距
        
        // 设置文档尺寸与页面完全匹配
        QPageLayout layout = printer.pageLayout();
        QRectF pageRect = layout.fullRect(QPageLayout::Point);
        
        // 设置放大样式
        doc.setDefaultStyleSheet(
            "body { font-family: 'Microsoft YaHei', Arial, sans-serif; font-size: 16pt; }"
            "h1 { font-size: 28pt; }"
            "th { font-size: 18pt; }"
            "td { font-size: 18pt; }"
            "p { font-size: 16pt; }"
        );
        
        doc.setHtml(htmlContent);
        
        // 设置文档大小为打印区域大小
        doc.setPageSize(pageRect.size());
        
        // 打印文档
        doc.print(&printer);
        
        qDebug() << "PDF页面尺寸: " << pageRect.width() << "x" << pageRect.height() << "点";
        qDebug() << "已创建PDF文件:" << pdfPath;
        
        // 打开PDF文件
        QDesktopServices::openUrl(QUrl::fromLocalFile(pdfPath));
        
        return true;
    } catch (const std::exception& e) {
        qDebug() << "打印过程中发生异常:" << e.what();
        return false;
    } catch (...) {
        qDebug() << "打印过程中发生未知异常";
        return false;
    }
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
    // 生成唯一交易ID
    QString uuid = transactionId.isEmpty() ? 
                  QUuid::createUuid().toString(QUuid::WithoutBraces).mid(0, 10) : 
                  transactionId;
    
    // 构建完整的HTML文档
    QString html = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>ATM回单</title>\n"
        "<style>\n"
        "body { font-family: 'Microsoft YaHei', Arial, sans-serif; font-size: 14pt; margin: 20px; padding: 10px; }\n"
        "h1 { font-size: 24pt; text-align: center; margin-bottom: 20px; font-weight: bold; }\n"
        "table { width: 100%; border-collapse: collapse; margin: 20px 0; }\n"
        "th, td { padding: 12px; text-align: left; border-bottom: 1px solid #ddd; }\n"
        "th { font-weight: bold; width: 35%; font-size: 16pt; }\n"
        "td { font-size: 16pt; }\n"
        ".header, .footer { text-align: center; margin: 25px 0; }\n"
        ".divider { border-top: 3px solid black; margin: 20px 0; }\n"
        ".footer p { font-size: 14pt; margin-top: 20px; line-height: 1.5; }\n"
        ".amount { font-weight: bold; color: #c00000; }\n"
        ".receipt-title { font-size: 18pt; text-align: center; margin: 10px 0 20px 0; font-weight: bold; }\n"
        "</style>\n"
        "</head>\n"
        "<body>\n"
        "<div class='header'>\n"
        "<h1>" + bankName + "</h1>\n"
        "<div class='receipt-title'>交易回单</div>\n"
        "</div>\n"
        "<div class='divider'></div>\n"
        "<table>\n"
        "<tr><th>交易类型:</th><td>" + transactionType + "</td></tr>\n"
        "<tr><th>交易时间:</th><td>" + transactionDate.toString("yyyy-MM-dd hh:mm:ss") + "</td></tr>\n"
        "<tr><th>交易卡号:</th><td>尾号" + cardNumber.right(4) + "</td></tr>\n"
        "<tr><th>持卡人:</th><td>" + holderName + "</td></tr>\n"
        "<tr><th>交易金额:</th><td class='amount'>￥" + QString::number(amount, 'f', 2) + "</td></tr>\n"
        "<tr><th>交易后余额:</th><td>￥" + QString::number(balanceAfter, 'f', 2) + "</td></tr>\n";
    
    // 如果是转账，添加目标账户信息
    if (transactionType == "转账" && !targetCardNumber.isEmpty()) {
        html += "<tr><th>收款卡号:</th><td>尾号" + targetCardNumber.right(4) + "</td></tr>\n";
        if (!targetCardHolder.isEmpty()) {
            html += "<tr><th>收款人:</th><td>" + targetCardHolder + "</td></tr>\n";
        }
    }
    
    html += "<tr><th>交易编号:</th><td>" + uuid + "</td></tr>\n"
            "</table>\n"
            "<div class='divider'></div>\n"
            "<div class='footer'>\n"
            "<p>此回单作为交易凭证，请妥善保管。<br>\n"
            "感谢您使用ATM模拟器银行服务！</p>\n"
            "</div>\n"
            "</body>\n"
            "</html>";
    
    return html;
}