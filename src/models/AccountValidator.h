/**
 * @file AccountValidator.h
 * @brief 账户验证器类
 *
 * 集中处理所有与账户操作相关的验证逻辑。
 */
#pragma once

#include <QString>
#include "IAccountRepository.h"
#include "OperationResult.h"

/**
 * @brief 账户验证器类
 *
 * 负责验证所有账户操作的合法性，如登录、取款、存款等。
 * 集中管理验证逻辑，确保验证规则的一致性。
 */
class AccountValidator {
public:
    /**
     * @brief 构造函数
     * @param repository 账户存储库接口指针
     */
    explicit AccountValidator(IAccountRepository* repository);
    
    /**
     * @brief 验证账户凭据
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 操作结果
     */
    OperationResult validateCredentials(const QString& cardNumber, const QString& pin) const;
    
    /**
     * @brief 验证管理员登录
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 操作结果
     */
    OperationResult validateAdminLogin(const QString& cardNumber, const QString& pin) const;
    
    /**
     * @brief 验证取款操作
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果
     */
    OperationResult validateWithdrawal(const QString& cardNumber, double amount) const;
    
    /**
     * @brief 验证存款操作
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果
     */
    OperationResult validateDeposit(const QString& cardNumber, double amount) const;
    
    /**
     * @brief 验证转账操作
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果
     */
    OperationResult validateTransfer(const QString& fromCardNumber, 
                                     const QString& toCardNumber, 
                                     double amount) const;
    
    /**
     * @brief 验证目标账户
     * @param targetCardNumber 目标卡号
     * @return 操作结果
     */
    OperationResult validateTargetAccount(const QString& targetCardNumber) const;
    
    /**
     * @brief 验证PIN码修改
     * @param cardNumber 卡号
     * @param currentPin 当前PIN码
     * @param newPin 新PIN码
     * @param confirmPin 确认新PIN码
     * @return 操作结果
     */
    OperationResult validatePinChange(const QString& cardNumber, 
                                      const QString& currentPin,
                                      const QString& newPin, 
                                      const QString& confirmPin = QString()) const;
    
    /**
     * @brief 验证管理员操作
     * @param cardNumber 管理员卡号
     * @return 操作结果
     */
    OperationResult validateAdminOperation(const QString& cardNumber) const;
    
    /**
     * @brief 验证创建账户操作
     * @param cardNumber 新卡号
     * @param pin PIN码
     * @param holderName 持卡人姓名
     * @param balance 初始余额
     * @param withdrawLimit 取款限额
     * @param isAdmin 是否为管理员
     * @return 操作结果
     */
    OperationResult validateCreateAccount(const QString& cardNumber, 
                                         const QString& pin, 
                                         const QString& holderName,
                                         double balance, 
                                         double withdrawLimit, 
                                         bool isAdmin) const;
    
    /**
     * @brief 验证更新账户操作
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 余额
     * @param withdrawLimit 取款限额
     * @return 操作结果
     */
    OperationResult validateUpdateAccount(const QString& cardNumber, 
                                         const QString& holderName,
                                         double balance, 
                                         double withdrawLimit) const;
    
    /**
     * @brief 验证PIN码格式
     * @param pin PIN码
     * @return 操作结果
     */
    OperationResult validatePinFormat(const QString& pin) const;
    
    /**
     * @brief 验证卡号格式
     * @param cardNumber 卡号
     * @return 操作结果
     */
    OperationResult validateCardNumberFormat(const QString& cardNumber) const;
    
private:
    //!< 账户存储库接口指针
    IAccountRepository* m_repository;
}; 