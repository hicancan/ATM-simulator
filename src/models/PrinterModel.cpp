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
 * @param showPrintDialog 是否显示打印对话框 (在此实现中未使用，直接生成 PDF 并打开)
 * @return 如果打印成功返回 true，否则返回 false
 * @note 此实现直接生成 PDF 并打开，不弹出打印对话框。
 */
bool PrinterModel::printReceipt(const QString &htmlContent, bool showPrintDialog)
{
    Q_UNUSED(showPrintDialog); // 在这个实现中不使用 showPrintDialog 参数

    try {
        // 首先确定文档保存路径（例如：用户的文档目录）
        QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        if (!QDir(documentsPath).exists()) {
            QDir().mkpath(documentsPath); // 如果目录不存在则创建
        }

        // 创建带有时间戳的唯一文件名
        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
        QString pdfPath = documentsPath + "/ATM_Receipt_" + timestamp + ".pdf";
        // 不再需要单独保存 HTML 文件，直接用于生成 PDF

        // 使用 QPdfWriter 直接创建 PDF 文件
        QPdfWriter pdfWriter(pdfPath);

        // 设置 PDF writer 的页面属性
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setPageOrientation(QPageLayout::Portrait);
        // 关键修复: 设置更小的页面边距，充分利用整个页面
        pdfWriter.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
        // 使用 QPdfWriter 时的分辨率通常由 writer 本身控制，这里设置 dpi
        pdfWriter.setResolution(m_printer->resolution()); // 使用打印机分辨率

        // 创建文档并设置合适的样式和内容
        QTextDocument doc;
        doc.setDocumentMargin(0); // 减少文档内边距

        // 设置文档尺寸与 PDF Writer 的页面完全匹配
        // 注意：QPdfWriter 的 pageRect() 返回的是像素单位
        // QPageLayout 的 fullRect 返回的是 point 单位
        // 为了文档内容正确缩放，最好设置 doc 的 pageSize 与 writer 的 pageSize 匹配，
        // 或者让 doc 自己去适应 layout
        QPageLayout layout = pdfWriter.pageLayout();
        QRectF pageRectPoints = layout.fullRect(QPageLayout::Point);
        doc.setPageSize(pageRectPoints.size());


        // 设置放大样式 (针对 PDF 输出可能需要调整字体大小以适应 A4)
        doc.setDefaultStyleSheet(
            "body { font-family: 'Microsoft YaHei', Arial, sans-serif; font-size: 12pt; }" // 调整基础字体大小
            "h1 { font-size: 20pt; }" // 调整标题大小
            "th { font-size: 14pt; }" // 调整表头字体大小
            "td { font-size: 14pt; }" // 调整表格数据字体大小
            "p { font-size: 12pt; }" // 调整段落字体大小
            ".receipt-title { font-size: 16pt; }" // 调整回单标题大小
        );

        doc.setHtml(htmlContent);

        // 使用 QPainter 绘制文档到 PDF Writer
        QPainter painter(&pdfWriter);
        doc.drawContents(&painter);

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
                  QUuid::createUuid().toString(QUuid::WithoutBraces).mid(0, 10).toUpper() : // 生成简短的 UUID，转换为大写
                  transactionId;

    // 构建完整的 HTML 文档
    QString html =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "<meta charset=\"UTF-8\">\n"
        "<title>ATM 回单</title>\n" // 调整标题
        "<style>\n"
        "body { font-family: 'Microsoft YaHei', Arial, sans-serif; font-size: 10pt; margin: 15px; padding: 8px; }\n" // 调整基础样式
        "h1 { font-size: 20pt; text-align: center; margin-bottom: 15px; font-weight: bold; }\n" // 调整标题样式
        "table { width: 100%; border-collapse: collapse; margin: 15px 0; }\n" // 调整表格样式
        "th, td { padding: 10px; text-align: left; border-bottom: 1px solid #ddd; }\n" // 调整表格单元格样式
        "th { font-weight: bold; width: 40%; font-size: 12pt; }\n" // 调整表头样式
        "td { font-size: 12pt; }\n" // 调整表格数据样式
        ".header, .footer { text-align: center; margin: 20px 0; }\n" // 调整头部和尾部样式
        ".divider { border-top: 2px dashed black; margin: 15px 0; }\n" // 调整分割线样式
        ".footer p { font-size: 10pt; margin-top: 15px; line-height: 1.4; }\n" // 调整尾部段落样式
        ".amount { font-weight: bold; color: #c00000; }\n" // 调整金额样式
        ".receipt-title { font-size: 14pt; text-align: center; margin: 8px 0 15px 0; font-weight: bold; }\n" // 调整回单标题样式
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
        "<tr><th>交易卡号:</th><td>尾号" + cardNumber.right(4) + "</td></tr>\n" // 只显示卡号后四位
        "<tr><th>持卡人:</th><td>" + holderName + "</td></tr>\n"
        "<tr><th>交易金额:</th><td class='amount'>￥" + QString::number(amount, 'f', 2) + "</td></tr>\n" // 格式化金额
        "<tr><th>交易后余额:</th><td>￥" + QString::number(balanceAfter, 'f', 2) + "</td></tr>\n"; // 格式化余额

    // 如果是转账，添加目标账户信息
    if (transactionType == "转账" && !targetCardNumber.isEmpty()) {
        html += "<tr><th>收款卡号:</th><td>尾号" + targetCardNumber.right(4) + "</td></tr>\n"; // 只显示目标卡号后四位
        if (!targetCardHolder.isEmpty()) {
            html += "<tr><th>收款人:</th><td>" + targetCardHolder + "</td></tr>\n";
        }
    }

    html += "<tr><th>交易编号:</th><td>" + uuid + "</td></tr>\n" // 添加交易编号
            "</table>\n"
            "<div class='divider'></div>\n"
            "<div class='footer'>\n"
            "<p>此回单作为交易凭证，请妥善保管。<br>\n" // 添加底部提示信息
            "感谢您使用 " + bankName + " ATM 模拟器银行服务！</p>\n" // 个性化感谢信息
            "</div>\n"
            "</body>\n"
            "</html>";

    return html; // 返回生成的 HTML 字符串
}