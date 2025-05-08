/**
 * @file AccountService.h
 * @brief 账户服务类
 *
 * 提供核心账户操作服务，处理取款、存款、转账等基本功能。
 */
#pragma once

#include <QString>
#include "IAccountRepository.h"
#include "AccountValidator.h"
#include "TransactionModel.h"
#include "LoginResult.h"
#include "OperationResult.h"

/**
 * @brief 账户服务类
 *
 * 实现与用户账户相关的核心业务逻辑，如登录、取款、存款、转账等。
 * 使用AccountValidator进行输入验证，通过IAccountRepository访问数据。
 */
class AccountService {
public:
    /**
     * @brief 构造函数
     * @param repository 账户存储库
     * @param validator 账户验证器
     * @param transactionModel 交易记录模型（可选）
     */
    AccountService(IAccountRepository* repository, 
                  AccountValidator* validator,
                  TransactionModel* transactionModel = nullptr);
    
    /**
     * @brief 设置交易记录模型
     * @param transactionModel 交易记录模型
     */
    void setTransactionModel(TransactionModel* transactionModel);
    
    /**
     * @brief 执行用户登录
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 登录结果，包含成功状态和账户信息
     */
    LoginResult performLogin(const QString& cardNumber, const QString& pin);
    
    /**
     * @brief 执行取款操作
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果
     */
    OperationResult withdrawAmount(const QString& cardNumber, double amount);
    
    /**
     * @brief 执行存款操作
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果
     */
    OperationResult depositAmount(const QString& cardNumber, double amount);
    
    /**
     * @brief 执行转账操作
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果
     */
    OperationResult transferAmount(const QString& fromCardNumber, 
                                  const QString& toCardNumber, 
                                  double amount);
    
    /**
     * @brief 修改PIN码
     * @param cardNumber 卡号
     * @param currentPin 当前PIN码
     * @param newPin 新PIN码
     * @param confirmPin 确认新PIN码
     * @return 操作结果
     */
    OperationResult changePin(const QString& cardNumber, 
                             const QString& currentPin, 
                             const QString& newPin, 
                             const QString& confirmPin = QString());
    
    /**
     * @brief 获取账户余额
     * @param cardNumber 卡号
     * @return 账户余额
     */
    double getBalance(const QString& cardNumber) const;
    
    /**
     * @brief 获取持卡人姓名
     * @param cardNumber 卡号
     * @return 持卡人姓名
     */
    QString getHolderName(const QString& cardNumber) const;
    
    /**
     * @brief 获取账户取款限额
     * @param cardNumber 卡号
     * @return 取款限额
     */
    double getWithdrawLimit(const QString& cardNumber) const;
    
    /**
     * @brief 检查账户是否被锁定
     * @param cardNumber 卡号
     * @return 如果账户被锁定返回true，否则返回false
     */
    bool isAccountLocked(const QString& cardNumber) const;

private:
    /**
     * @brief 记录交易
     * @param cardNumber 交易涉及的卡号
     * @param type 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param description 交易描述
     * @param targetCard 目标卡号 (转账时使用)
     */
    void recordTransaction(const QString& cardNumber, 
                          TransactionType type,
                          double amount, 
                          double balanceAfter,
                          const QString& description, 
                          const QString& targetCard = QString());
    
    //!< 账户存储库
    IAccountRepository* m_repository;
    
    //!< 账户验证器
    AccountValidator* m_validator;
    
    //!< 交易记录模型
    TransactionModel* m_transactionModel;
}; 