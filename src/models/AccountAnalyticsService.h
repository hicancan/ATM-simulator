// AccountAnalyticsService.h
/**
 * @file AccountAnalyticsService.h
 * @brief 账户分析服务类
 *
 * 提供账户分析功能，如余额预测和交易趋势分析。
 */
#pragma once

#include <QString>
#include <QDateTime>
#include <QMap>
#include <QVector>
#include "IAccountRepository.h"
#include "TransactionModel.h"
#include "OperationResult.h"

/**
 * @brief 账户分析服务类
 *
 * 实现与账户分析相关的功能，包括余额预测和交易趋势分析。
 */
class AccountAnalyticsService {
public:
    /**
     * @brief 构造函数
     * @param repository 账户存储库
     * @param transactionModel 交易记录模型
     */
    AccountAnalyticsService(IAccountRepository* repository, 
                           TransactionModel* transactionModel);
    
    /**
     * @brief 预测未来余额
     * 
     * 根据历史交易记录预测未来一定天数后的余额。
     *
     * @param cardNumber 卡号
     * @param daysInFuture 预测未来天数（默认为7天）
     * @return 预测的余额，如果无法预测返回当前余额
     */
    double predictBalance(const QString& cardNumber, int daysInFuture = 7) const;
    
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
    OperationResult calculatePredictedBalance(const QString& cardNumber,
                                             int daysInFuture,
                                             double& outBalance) const;
    
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
    OperationResult predictBalanceMultiDays(const QString& cardNumber,
                                           const QVector<int>& days,
                                           QMap<int, double>& outPredictions) const;
    
    /**
     * @brief 使用线性回归模型预测未来余额
     * 
     * 使用线性回归分析历史交易趋势，进行更精确的余额预测。
     *
     * @param cardNumber 卡号
     * @param daysInFuture 预测未来天数
     * @return 预测的余额，如果无法预测返回当前余额
     */
    double predictBalanceWithRegression(const QString& cardNumber, int daysInFuture) const;

    /**
     * @brief 使用加权平均模型预测未来余额
     * 
     * 使用加权平均方法，对近期交易赋予更高权重，进行更精确的余额预测。
     *
     * @param cardNumber 卡号
     * @param daysInFuture 预测未来天数
     * @return 预测的余额，如果无法预测返回当前余额
     */
    double predictBalanceWithWeightedAverage(const QString& cardNumber, int daysInFuture) const;
    
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
    OperationResult getAccountTrend(const QString& cardNumber,
                                   int days,
                                   QMap<QDate, double>& outIncomeTrend,
                                   QMap<QDate, double>& outExpenseTrend) const;
    
    /**
     * @brief 获取交易活跃度
     * 
     * 计算账户交易频率，分析用户活跃度。
     *
     * @param cardNumber 卡号
     * @param days 分析天数
     * @return 平均每天交易次数
     */
    double getTransactionFrequency(const QString& cardNumber, int days = 30) const;

private:
    /**
     * @brief 根据历史交易计算日均收支
     * @param transactions 交易记录列表
     * @param days 分析天数
     * @param outDailyIncome 输出参数，日均收入
     * @param outDailyExpense 输出参数，日均支出
     */
    void calculateDailyAverages(const QVector<Transaction>& transactions,
                               int days,
                               double& outDailyIncome,
                               double& outDailyExpense) const;
                               
    /**
     * @brief 计算线性回归参数
     * @param xValues x值列表（时间点）
     * @param yValues y值列表（余额变化）
     * @param outSlope 输出参数，回归直线斜率
     * @param outIntercept 输出参数，回归直线截距
     */
    void calculateLinearRegression(const QVector<double>& xValues,
                                  const QVector<double>& yValues,
                                  double& outSlope,
                                  double& outIntercept) const;
    
    //!< 账户存储库
    IAccountRepository* m_repository;
    
    //!< 交易记录模型
    TransactionModel* m_transactionModel;
}; 