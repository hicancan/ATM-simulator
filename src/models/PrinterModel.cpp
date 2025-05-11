// PrinterModel.cpp
/**
 * @file PrinterModel.cpp
 * @brief 打印模型实现文件
 *
 * 实现了 PrinterModel 类中定义的打印内容生成和实际打印方法。
 * 处理与打印机和文件相关的操作。
 */
#include "PrinterModel.h"
#include <QDebug>
#include <QGuiApplication>
#include <QUuid>
#include <QFileDialog>
#include <QPdfWriter> // 用于直接生成 PDF
#include <QPainter> // 用于 QPdfWriter
#include <QDesktopServices> // 用于打开文件
#include <QUrl> // 用于文件 URL
#include <QDir> // 用于目录操作
#include <QStandardPaths> // 用于获取标准目录
#include <QTemporaryFile> // 用于临时文件
#include <QTextStream> // 用于文本流
#include <QFile> // 用于文件操作
#include <QPrinterInfo> // 用于获取打印机信息

/**
 * @brief 构造函数
 * @param parent 父对象
 */
PrinterModel::PrinterModel(QObject *parent)
    : QObject(parent)
    , m_printer(nullptr) //!< 初始化打印机指针为空
{
    initializePrinter(); // 在构造函数中初始化打印机
}

/**
 * @brief 析构函数
 *
 * 销毁 QPrinter 对象。
 */
PrinterModel::~PrinterModel()
{
    if (m_printer) {
        delete m_printer;
        m_printer = nullptr;
    }
}

/**
 * @brief 初始化打印机设置
 *
 * 配置 QPrinter 对象，如纸张大小、方向和默认打印机。
 */
void PrinterModel::initializePrinter()
{
    if (!m_printer) {
        m_printer = new QPrinter(QPrinter::HighResolution); // 创建高分辨率打印机对象

        // 使用标准 A4 纸张大小
        m_printer->setPageSize(QPageSize(QPageSize::A4));
        m_printer->setPageOrientation(QPageLayout::Portrait); // 设置纵向

        // 获取默认打印机并设置
        QString defaultPrinterName = QPrinterInfo::defaultPrinterName();
        if (!defaultPrinterName.isEmpty()) {
            m_printer->setPrinterName(defaultPrinterName);
            qDebug() << "设置默认打印机: " << defaultPrinterName;
        } else {
            qDebug() << "未找到默认打印机";
            // 如果没有默认打印机，打印可能会失败或需要用户手动选择
        }
    }
}

/**
 * @brief 打印回单
 *
 * 将生成的 HTML 内容通过 QPrinter 转换为 PDF 并打开。
 *
 * @param htmlContent 回单的 HTML 内容
 * @return 如果打印成功返回 true，否则返回 false
 * @note 此实现直接生成 PDF 并打开，不弹出打印对话框。
 */
bool PrinterModel::printReceipt(const QString &htmlContent)
{
    try {
        // 首先确定文档保存路径（例如：用户的文档目录）
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (!QDir(documentsPath).exists()) {
            QDir().mkpath(documentsPath); // 如果目录不存在则创建
        }

        // 创建带有时间戳的唯一文件名
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString pdfPath = documentsPath + "/ATM_Receipt_" + timestamp + ".pdf";
        
        // 使用 QPdfWriter 直接创建 PDF 文件
        QPdfWriter pdfWriter(pdfPath);

        // 设置 PDF writer 的页面属性
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setPageOrientation(QPageLayout::Portrait);
        // 将边距设置为最小值，最大化内容区域
        pdfWriter.setPageMargins(QMarginsF(5, 5, 5, 5), QPageLayout::Millimeter);
        // 设置更高分辨率
        pdfWriter.setResolution(600);

        // 创建文档
        QTextDocument doc;
        
        // 创建更全面的HTML文档结构，修改样式以填充更多空间
        QString enhancedHtml = QString(
            "<html>"
            "<head>"
            "<style type='text/css'>"
            "body { font-family: 'Microsoft YaHei', Arial, sans-serif; text-align: center; margin: 0; padding: 0; color: #000000; width: 100%; }"
            "table { width: 100%; margin: 10px auto; border-collapse: collapse; }"
            "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; font-size: 14pt; color: #000000; }"
            "th { font-weight: bold; width: 40%; color: #000000; }"
            ".amount { font-weight: bold; color: #c00000; }"
            ".header { margin-bottom: 10px; width: 100%; color: #000000; }"
            ".footer { margin-top: 10px; width: 100%; color: #000000; }"
            ".divider { border-top: 2px solid black; margin: 10px auto; width: 100%; }"
            "h2, h3 { margin: 5px 0; color: #000000; }"
            "p { color: #000000; font-size: 12pt; }"
            "div { color: #000000; }"
            "#main-container { width: 100%; margin: 0 auto; padding: 0; }"
            "</style>"
            "</head>"
            "<body><div id='main-container'>" + htmlContent + "</div></body>"
            "</html>"
        );
        
        // 设置文档内容
        doc.setHtml(enhancedHtml);
        
        // 设置文档默认样式表，确保所有文本为黑色
        doc.setDefaultStyleSheet("* { color: #000000; }");
        
        // 设置文档尺寸为A4大小（以点为单位，1点 = 1/72英寸）
        // A4大小约为595×842点
        doc.setPageSize(QSizeF(595, 842));
        
        // 确保文档宽度与PDF页面宽度匹配
        doc.setTextWidth(pdfWriter.width());
        
        // 使用QPainter绘制文档到PDF
        QPainter painter(&pdfWriter);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::TextAntialiasing);
        
        // 绘制文档内容，确保填充整个页面
        doc.drawContents(&painter);
        painter.end();

        qDebug() << "PDF 已创建:" << pdfPath;

        // 打开生成的 PDF 文件
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

/**
 * @brief 生成回单的 HTML 内容
 *
 * 根据交易详情生成用于打印的 HTML 格式回单内容。
 *
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
 * @return 生成的 HTML 内容字符串
 */
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
    // 生成唯一交易 ID，如果未提供则生成新的 UUID
    QString uuid = transactionId.isEmpty() ?
                  QUuid::createUuid().toString(QUuid::WithoutBraces).mid(0, 10).toUpper() :
                  transactionId;

    // 构建改进的HTML结构，确保所有信息清晰可见并设置明确的黑色文本颜色
    QString html = 
        // 头部 - 减小高度，保持样式
        "<div style='text-align: center; width: 100%; padding: 10px 0; border-bottom: 2px solid #000; color: #000000; background-color: #f8f8f8;'>"
        "<h2 style='font-size: 22pt; margin: 2px 0; color: #000000;'>" + bankName + "</h2>"
        "<h3 style='font-size: 16pt; margin: 2px 0; color: #000000;'>交易回单</h3>"
        "</div>"
        
        // 交易信息表格 - 扩大表格区域，增加内容比例
        "<table style='width: 100%; margin: 40px auto; font-size: 16pt; color: #000000; border: 1px solid #ddd;'>"
        "<tr><th style='width: 35%; text-align: left; padding: 12px; color: #000000; background-color: #f0f0f0;'>交易类型:</th><td style='padding: 12px; color: #000000;'><strong>" + transactionType + "</strong></td></tr>"
        "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>交易时间:</th><td style='padding: 12px; color: #000000;'>" + transactionDate.toString("yyyy-MM-dd hh:mm:ss") + "</td></tr>"
        "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>交易卡号:</th><td style='padding: 12px; color: #000000;'>尾号" + cardNumber.right(4) + "</td></tr>"
        "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>持卡人:</th><td style='padding: 12px; color: #000000;'>" + holderName + "</td></tr>"
        "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>交易金额:</th><td style='padding: 12px; font-weight: bold; color: #c00000; font-size: 18pt;'>￥" + QString::number(amount, 'f', 2) + "</td></tr>"
        "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>交易后余额:</th><td style='padding: 12px; color: #000000;'>￥" + QString::number(balanceAfter, 'f', 2) + "</td></tr>";

    // 如果是转账，添加目标账户信息
    if (transactionType == "转账" && !targetCardNumber.isEmpty()) {
        html += "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>收款卡号:</th><td style='padding: 12px; color: #000000;'>尾号" + targetCardNumber.right(4) + "</td></tr>";
        if (!targetCardHolder.isEmpty()) {
            html += "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>收款人:</th><td style='padding: 12px; color: #000000;'>" + targetCardHolder + "</td></tr>";
        }
    }

    html += "<tr><th style='padding: 12px; color: #000000; background-color: #f0f0f0;'>交易编号:</th><td style='padding: 12px; color: #000000;'>" + uuid + "</td></tr>"
            "</table>"
            
            // 分隔线
            "<div style='border-top: 2px solid #000; width: 100%; margin: 40px 0;'></div>"
            
            // 底部信息 - 减小高度
            "<div style='text-align: center; margin-top: 0; width: 100%; font-size: 12pt; color: #000000; background-color: #f8f8f8; padding: 10px 0;'>"
            "<p style='margin: 2px 0; color: #000000;'>此回单作为交易凭证，请妥善保管。</p>"
            "<p style='margin: 2px 0; color: #000000;'>感谢您使用 " + bankName + " ATM 模拟器银行服务！</p>"
            "<p style='margin: 2px 0; color: #000000;'>" + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " 打印</p>"
            "</div>";

    return html;
}