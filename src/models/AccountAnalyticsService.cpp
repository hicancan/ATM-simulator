// AccountAnalyticsService.cpp
/**
 * @file AccountAnalyticsService.cpp
 * @brief 账户分析服务类实现
 *
 * 实现了AccountAnalyticsService类中定义的分析方法。
 */
#include "AccountAnalyticsService.h"
#include <QDebug>
#include <algorithm>
#include <numeric>

/**
 * @brief 构造函数
 * @param repository 账户存储库
 * @param transactionModel 交易记录模型
 */
AccountAnalyticsService::AccountAnalyticsService(IAccountRepository* repository, 
                                               TransactionModel* transactionModel)
    : m_repository(repository)
    , m_transactionModel(transactionModel)
{
}

/**
 * @brief 预测未来余额
 * 
 * 根据历史交易记录预测未来一定天数后的余额。
 *
 * @param cardNumber 卡号
 * @param daysInFuture 预测未来天数
 * @return 预测的余额，如果无法预测返回当前余额
 */
double AccountAnalyticsService::predictBalance(const QString& cardNumber, int daysInFuture) const
{
    if (!m_transactionModel) {
        qWarning() << "TransactionModel 为空，无法预测余额。";
        std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
        return accountOpt ? accountOpt.value().balance : 0.0; // 如果没有交易模型，返回当前余额
    }

    // 获取交易记录
    QVector<Transaction> transactions = m_transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.size() < 2) {
        qWarning() << "交易记录不足，无法预测卡号为:" << cardNumber << " 的余额。";
        std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
        return accountOpt ? accountOpt.value().balance : 0.0; // 如果交易数据不足，返回当前余额
    }

    // 按日期排序交易记录（最新的在后）
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp < b.timestamp;
    });

    // 使用最近的 N 条交易记录进行趋势预测 (例如，最近 10 条或更少)
    const int maxTransactionsForAnalysis = std::min(10, static_cast<int>(transactions.size()));
    const int startIdx = transactions.size() - maxTransactionsForAnalysis;
    
    // 计算日均收入和支出
    double dailyIncome = 0.0;
    double dailyExpense = 0.0;
    calculateDailyAverages(
        transactions.mid(startIdx, maxTransactionsForAnalysis),
        30, // 使用最近30天的数据来计算平均值
        dailyIncome,
        dailyExpense
    );

    // 获取当前余额
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return 0.0;
    }
    double currentBalance = accountOpt.value().balance;
    
    // 预测未来余额：当前余额 + (日均收入 - 日均支出) * 未来天数
    double predictedDailyChange = dailyIncome - dailyExpense;
    double predictedBalance = currentBalance + (predictedDailyChange * daysInFuture);
    
    // 确保预测余额不为负
    if (predictedBalance < 0) {
        predictedBalance = 0;
    }
    
    qDebug() << "账户:" << cardNumber 
             << "当前余额:" << currentBalance 
             << "预测" << daysInFuture << "天后余额:" << predictedBalance;
    
    return predictedBalance;
}

/**
 * @brief 计算预测余额的结果封装
 * 
 * 计算预测余额，并通过参数返回预测结果，同时返回操作结果。
 *
 * @param cardNumber 卡号
 * @param daysInFuture 预测未来天数
 * @param outBalance 输出参数，保存预测的余额
 * @return 操作结果
 */
OperationResult AccountAnalyticsService::calculatePredictedBalance(const QString& cardNumber,
                                                                 int daysInFuture,
                                                                 double& outBalance) const
{
    // 验证输入参数
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    if (daysInFuture <= 0) {
        return OperationResult::Failure("预测天数必须为正数");
    }
    
    // 检查账户是否存在
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 检查交易模型是否可用
    if (!m_transactionModel) {
        outBalance = accountOpt.value().balance;
        return OperationResult::Failure("交易数据模型不可用，返回当前余额");
    }
    
    // 计算预测余额
    outBalance = predictBalance(cardNumber, daysInFuture);
    
    return OperationResult::Success();
}

/**
 * @brief 获取账户收支趋势
 * 
 * 分析账户历史交易，提供收入和支出的趋势数据。
 *
 * @param cardNumber 卡号
 * @param days 分析天数
 * @param outIncomeTrend 输出参数，收入趋势
 * @param outExpenseTrend 输出参数，支出趋势
 * @return 操作结果
 */
OperationResult AccountAnalyticsService::getAccountTrend(const QString& cardNumber,
                                                       int days,
                                                       QMap<QDate, double>& outIncomeTrend,
                                                       QMap<QDate, double>& outExpenseTrend) const
{
    // 验证输入参数
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    if (days <= 0) {
        return OperationResult::Failure("分析天数必须为正数");
    }
    
    // 检查账户是否存在
    if (!m_repository->accountExists(cardNumber)) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 检查交易模型是否可用
    if (!m_transactionModel) {
        return OperationResult::Failure("交易数据模型不可用");
    }
    
    // 获取交易记录
    QVector<Transaction> transactions = m_transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.isEmpty()) {
        return OperationResult::Failure("没有可用的交易记录");
    }
    
    // 清空输出映射
    outIncomeTrend.clear();
    outExpenseTrend.clear();
    
    // 计算起始日期
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-days + 1); // +1 包含今天
    
    // 初始化日期范围内的所有日期为0
    for (QDate date = startDate; date <= endDate; date = date.addDays(1)) {
        outIncomeTrend[date] = 0.0;
        outExpenseTrend[date] = 0.0;
    }
    
    // 按日期对交易进行分组和汇总
    for (const auto& transaction : transactions) {
        QDate transactionDate = transaction.timestamp.date();
        
        // 只考虑指定日期范围内的交易
        if (transactionDate >= startDate && transactionDate <= endDate) {
            // 根据交易类型分类为收入或支出
            if (transaction.type == TransactionType::Deposit) {
                // 存款视为收入
                outIncomeTrend[transactionDate] += transaction.amount;
            } else if (transaction.type == TransactionType::Withdrawal || 
                      transaction.type == TransactionType::Transfer) {
                // 取款和转账视为支出
                outExpenseTrend[transactionDate] += transaction.amount;
            }
        }
    }
    
    return OperationResult::Success();
}

/**
 * @brief 获取交易活跃度
 * 
 * 计算账户交易频率，分析用户活跃度。
 *
 * @param cardNumber 卡号
 * @param days 分析天数
 * @return 平均每天交易次数
 */
double AccountAnalyticsService::getTransactionFrequency(const QString& cardNumber, int days) const
{
    // 验证输入参数
    if (cardNumber.isEmpty() || days <= 0 || !m_transactionModel) {
        return 0.0;
    }
    
    // 检查账户是否存在
    if (!m_repository->accountExists(cardNumber)) {
        return 0.0;
    }
    
    // 获取交易记录
    QVector<Transaction> transactions = m_transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.isEmpty()) {
        return 0.0;
    }
    
    // 计算日期范围
    QDate endDate = QDate::currentDate();
    QDate startDate = endDate.addDays(-days + 1); // +1 包含今天
    
    // 统计日期范围内的交易次数
    int transactionCount = 0;
    for (const auto& transaction : transactions) {
        QDate transactionDate = transaction.timestamp.date();
        if (transactionDate >= startDate && transactionDate <= endDate) {
            transactionCount++;
        }
    }
    
    // 计算平均每天交易次数
    return static_cast<double>(transactionCount) / days;
}

/**
 * @brief 根据历史交易计算日均收支
 * @param transactions 交易记录列表
 * @param days 分析天数
 * @param outDailyIncome 输出参数，日均收入
 * @param outDailyExpense 输出参数，日均支出
 */
void AccountAnalyticsService::calculateDailyAverages(const QVector<Transaction>& transactions,
                                                   int days,
                                                   double& outDailyIncome,
                                                   double& outDailyExpense) const
{
    if (transactions.isEmpty() || days <= 0) {
        outDailyIncome = 0.0;
        outDailyExpense = 0.0;
        return;
    }
    
    double totalIncome = 0.0;
    double totalExpense = 0.0;
    
    // 计算总收入和总支出
    for (const auto& transaction : transactions) {
        if (transaction.type == TransactionType::Deposit) {
            // 存款视为收入
            totalIncome += transaction.amount;
        } else if (transaction.type == TransactionType::Withdrawal || 
                  transaction.type == TransactionType::Transfer) {
            // 取款和转账视为支出
            totalExpense += transaction.amount;
        }
    }
    
    // 计算日均值
    outDailyIncome = totalIncome / days;
    outDailyExpense = totalExpense / days;
} 