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
#include <cmath>

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
    // 优先使用加权平均方法进行预测
    return predictBalanceWithWeightedAverage(cardNumber, daysInFuture);
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
 * @brief 多日期预测余额
 * 
 * 预测未来多个日期点的余额变化趋势。
 *
 * @param cardNumber 卡号
 * @param days 预测天数数组，如[7, 14, 30, 90]
 * @param outPredictions 输出参数，保存各天数的预测余额，key为天数
 * @return 操作结果
 */
OperationResult AccountAnalyticsService::predictBalanceMultiDays(const QString& cardNumber,
                                                               const QVector<int>& days,
                                                               QMap<int, double>& outPredictions) const
{
    // 验证输入参数
    if (cardNumber.isEmpty()) {
        return OperationResult::Failure("卡号不能为空");
    }
    
    if (days.isEmpty()) {
        return OperationResult::Failure("预测天数列表不能为空");
    }
    
    // 检查账户是否存在
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return OperationResult::Failure("账户不存在");
    }
    
    // 检查交易模型是否可用
    if (!m_transactionModel) {
        // 如果交易数据模型不可用，所有预测结果都为当前余额
        double currentBalance = accountOpt.value().balance;
        for (int day : days) {
            outPredictions[day] = currentBalance;
        }
        return OperationResult::Failure("交易数据模型不可用，返回当前余额");
    }
    
    // 清空结果映射
    outPredictions.clear();
    
    // 针对每个指定天数进行预测
    for (int day : days) {
        if (day <= 0) {
            continue; // 跳过无效天数
        }
        outPredictions[day] = predictBalance(cardNumber, day);
    }
    
    return OperationResult::Success();
}

/**
 * @brief 使用线性回归模型预测未来余额
 * 
 * 使用线性回归分析历史交易趋势，进行更精确的余额预测。
 *
 * @param cardNumber 卡号
 * @param daysInFuture 预测未来天数
 * @return 预测的余额，如果无法预测返回当前余额
 */
double AccountAnalyticsService::predictBalanceWithRegression(const QString& cardNumber, int daysInFuture) const
{
    if (!m_transactionModel) {
        qWarning() << "TransactionModel 为空，无法预测余额。";
        std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
        return accountOpt ? accountOpt.value().balance : 0.0;
    }

    // 获取当前余额
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return 0.0;
    }
    double currentBalance = accountOpt.value().balance;
    
    // 获取交易记录
    QVector<Transaction> transactions = m_transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.size() < 5) { // 需要至少5条记录进行回归
        qWarning() << "交易记录不足，无法使用回归方法预测卡号为:" << cardNumber << " 的余额。";
        return currentBalance;
    }

    // 按日期排序交易记录（最新的在后）
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp < b.timestamp;
    });

    // 准备回归分析数据
    QVector<double> xValues; // 日期（转换为距今天数）
    QVector<double> yValues; // 每日余额
    
    // 基准日期为当前日期
    QDate currentDate = QDate::currentDate();
    
    // 每日余额记录
    QMap<QDate, double> dailyBalances;
    
    // 初始化初始日期和余额
    QDate firstTransactionDate = transactions.first().timestamp.date();
    double runningBalance = currentBalance;
    
    // 倒序计算历史余额
    for (int i = transactions.size() - 1; i >= 0; --i) {
        const Transaction& tx = transactions[i];
        QDate txDate = tx.timestamp.date();
        
        // 根据交易类型计算历史余额
        if (tx.type == TransactionType::Deposit) {
            runningBalance -= tx.amount; // 存款前余额会减少
        } else if (tx.type == TransactionType::Withdrawal || tx.type == TransactionType::Transfer) {
            runningBalance += tx.amount; // 取款或转账前余额会增加
        }
        
        // 记录该日期的余额
        dailyBalances[txDate] = runningBalance;
    }
    
    // 将余额数据转换为回归分析所需的格式
    for (auto it = dailyBalances.constBegin(); it != dailyBalances.constEnd(); ++it) {
        // x值为日期距离今天的天数
        int daysFromToday = it.key().daysTo(currentDate);
        xValues.append(daysFromToday);
        
        // y值为当日余额
        yValues.append(it.value());
    }
    
    // 如果数据点太少，回退到简单方法
    if (xValues.size() < 2) {
        return predictBalanceWithWeightedAverage(cardNumber, daysInFuture);
    }
    
    // 计算线性回归参数
    double slope = 0.0;
    double intercept = 0.0;
    calculateLinearRegression(xValues, yValues, slope, intercept);
    
    // 使用回归方程预测未来余额
    // x是未来距离今天的天数（负值表示未来的日期）
    double futureDays = -static_cast<double>(daysInFuture);
    double predictedBalance = slope * futureDays + intercept;
    
    // 确保预测余额不为负
    if (predictedBalance < 0) {
        predictedBalance = 0;
    }
    
    qDebug() << "线性回归预测: 账户:" << cardNumber 
             << "当前余额:" << currentBalance 
             << "预测" << daysInFuture << "天后余额:" << predictedBalance
             << "斜率:" << slope << "截距:" << intercept;
    
    return predictedBalance;
}

/**
 * @brief 使用加权平均模型预测未来余额
 * 
 * 使用加权平均方法，对近期交易赋予更高权重，进行更精确的余额预测。
 *
 * @param cardNumber 卡号
 * @param daysInFuture 预测未来天数
 * @return 预测的余额，如果无法预测返回当前余额
 */
double AccountAnalyticsService::predictBalanceWithWeightedAverage(const QString& cardNumber, int daysInFuture) const
{
    if (!m_transactionModel) {
        qWarning() << "TransactionModel 为空，无法预测余额。";
        std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
        return accountOpt ? accountOpt.value().balance : 0.0;
    }

    // 获取交易记录
    QVector<Transaction> transactions = m_transactionModel->getTransactionsForCard(cardNumber);
    if (transactions.size() < 2) {
        qWarning() << "交易记录不足，无法预测卡号为:" << cardNumber << " 的余额。";
        std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
        return accountOpt ? accountOpt.value().balance : 0.0;
    }

    // 按日期排序交易记录（最新的在后）
    std::sort(transactions.begin(), transactions.end(), [](const Transaction& a, const Transaction& b) {
        return a.timestamp < b.timestamp;
    });

    // 获取当前余额
    std::optional<Account> accountOpt = m_repository->findByCardNumber(cardNumber);
    if (!accountOpt) {
        return 0.0;
    }
    double currentBalance = accountOpt.value().balance;
    
    // 使用加权平均法分析交易数据
    // 最近的交易数据权重更高
    double totalIncome = 0.0;
    double totalExpense = 0.0;
    double totalIncomeWeight = 0.0;
    double totalExpenseWeight = 0.0;
    
    QDate currentDate = QDate::currentDate();
    
    // 分析过去90天的交易
    const int analysisPeriod = 90;
    QDate startDate = currentDate.addDays(-analysisPeriod);
    
    for (const auto& transaction : transactions) {
        QDate txDate = transaction.timestamp.date();
        
        // 只分析指定日期范围内的交易
        if (txDate >= startDate && txDate <= currentDate) {
            // 计算权重 - 越近的交易权重越高
            double daysAgo = txDate.daysTo(currentDate);
            double weight = 1.0 / (1.0 + daysAgo * 0.05); // 权重随时间衰减
            
            if (transaction.type == TransactionType::Deposit) {
                totalIncome += transaction.amount * weight;
                totalIncomeWeight += weight;
            } else if (transaction.type == TransactionType::Withdrawal ||
                      transaction.type == TransactionType::Transfer) {
                totalExpense += transaction.amount * weight;
                totalExpenseWeight += weight;
            }
        }
    }
    
    // 计算加权平均日收入和支出
    double dailyIncome = (totalIncomeWeight > 0) ? (totalIncome / totalIncomeWeight) / analysisPeriod : 0.0;
    double dailyExpense = (totalExpenseWeight > 0) ? (totalExpense / totalExpenseWeight) / analysisPeriod : 0.0;
    
    // 根据交易频率调整日收支
    double frequency = getTransactionFrequency(cardNumber, analysisPeriod);
    if (frequency > 0) {
        dailyIncome = dailyIncome * std::min(frequency, 1.0);
        dailyExpense = dailyExpense * std::min(frequency, 1.0);
    }
    
    // 预测未来余额
    double predictedDailyChange = dailyIncome - dailyExpense;
    double predictedBalance = currentBalance + (predictedDailyChange * daysInFuture);
    
    // 确保预测余额不为负
    if (predictedBalance < 0) {
        predictedBalance = 0;
    }
    
    qDebug() << "加权平均预测: 账户:" << cardNumber 
             << "当前余额:" << currentBalance 
             << "预测" << daysInFuture << "天后余额:" << predictedBalance
             << "日均收入:" << dailyIncome << "日均支出:" << dailyExpense;
    
    return predictedBalance;
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

/**
 * @brief 计算线性回归参数
 * @param xValues x值列表（时间点）
 * @param yValues y值列表（余额变化）
 * @param outSlope 输出参数，回归直线斜率
 * @param outIntercept 输出参数，回归直线截距
 */
void AccountAnalyticsService::calculateLinearRegression(const QVector<double>& xValues,
                                                      const QVector<double>& yValues,
                                                      double& outSlope,
                                                      double& outIntercept) const
{
    if (xValues.size() != yValues.size() || xValues.isEmpty()) {
        outSlope = 0.0;
        outIntercept = 0.0;
        return;
    }
    
    // 计算均值
    double xMean = std::accumulate(xValues.begin(), xValues.end(), 0.0) / xValues.size();
    double yMean = std::accumulate(yValues.begin(), yValues.end(), 0.0) / yValues.size();
    
    // 计算回归参数
    double numerator = 0.0;
    double denominator = 0.0;
    
    for (int i = 0; i < xValues.size(); ++i) {
        double xDiff = xValues[i] - xMean;
        numerator += xDiff * (yValues[i] - yMean);
        denominator += xDiff * xDiff;
    }
    
    // 防止除零错误
    if (denominator == 0.0) {
        outSlope = 0.0;
        outIntercept = yMean;
        return;
    }
    
    // 计算斜率和截距
    outSlope = numerator / denominator;
    outIntercept = yMean - outSlope * xMean;
} 