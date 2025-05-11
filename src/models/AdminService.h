// AdminService.h
/**
 * @file AdminService.h
 * @brief 管理员服务类
 *
 * 提供管理员专用功能，如账户管理、锁定账户等操作。
 */
#pragma once

#include <QString>
#include "IAccountRepository.h"
#include "AccountValidator.h"
#include "TransactionModel.h"
#include "LoginResult.h"
#include "OperationResult.h"

/**
 * @brief 管理员服务类
 *
 * 实现与管理员功能相关的业务逻辑，如创建账户、修改账户、设置锁定状态等。
 * 使用AccountValidator进行输入验证，通过IAccountRepository访问数据。
 */
class AdminService {
public:
    /**
     * @brief 构造函数
     * @param repository 账户存储库
     * @param validator 账户验证器
     * @param transactionModel 交易记录模型（可选）
     */
    AdminService(IAccountRepository* repository, 
                AccountValidator* validator,
                TransactionModel* transactionModel = nullptr);
    
    /**
     * @brief 设置交易记录模型
     * @param transactionModel 交易记录模型
     */
    void setTransactionModel(TransactionModel* transactionModel);
    
    /**
     * @brief 执行管理员登录
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 登录结果，包含成功状态和账户信息
     */
    LoginResult performAdminLogin(const QString& cardNumber, const QString& pin);
    
    /**
     * @brief 创建新账户
     * @param cardNumber 卡号
     * @param pin PIN码
     * @param holderName 持卡人姓名
     * @param balance 初始余额
     * @param withdrawLimit 取款限额
     * @param isAdmin 是否为管理员账户
     * @return 操作结果
     */
    OperationResult createAccount(const QString& cardNumber, 
                                 const QString& pin, 
                                 const QString& holderName,
                                 double balance, 
                                 double withdrawLimit, 
                                 bool isAdmin = false);
    
    /**
     * @brief 更新现有账户信息
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 账户余额
     * @param withdrawLimit 取款限额
     * @param isLocked 是否锁定账户
     * @return 操作结果
     */
    OperationResult updateAccount(const QString& cardNumber, 
                                 const QString& holderName,
                                 double balance, 
                                 double withdrawLimit,
                                 bool isLocked);
    
    /**
     * @brief 删除账户
     * @param cardNumber 要删除的账户卡号
     * @return 操作结果
     */
    OperationResult deleteAccount(const QString& cardNumber);
    
    /**
     * @brief 设置账户锁定状态
     * @param cardNumber 卡号
     * @param locked 是否锁定
     * @return 操作结果
     */
    OperationResult setAccountLockStatus(const QString& cardNumber, bool locked);
    
    /**
     * @brief 重置PIN码
     * @param cardNumber 卡号
     * @param newPin 新PIN码
     * @return 操作结果
     */
    OperationResult resetPin(const QString& cardNumber, const QString& newPin);
    
    /**
     * @brief 设置取款限额
     * @param cardNumber 卡号
     * @param limit 新限额
     * @return 操作结果
     */
    OperationResult setWithdrawLimit(const QString& cardNumber, double limit);
    
    /**
     * @brief 获取所有账户列表
     * @return 所有账户的列表
     */
    QVector<Account> getAllAccounts() const;

    /**
     * @brief 检查管理员权限
     * @param cardNumber 卡号
     * @return 操作结果
     */
    OperationResult checkAdminPermission(const QString& cardNumber) const;

private:
    /**
     * @brief 记录管理员操作日志
     * @param adminCardNumber 管理员卡号
     * @param operationType 操作类型
     * @param targetCardNumber 目标卡号
     * @param description 操作描述
     */
    void logAdminOperation(const QString& adminCardNumber, 
                          const QString& operationType,
                          const QString& targetCardNumber, 
                          const QString& description);
    
    //!< 账户存储库
    IAccountRepository* m_repository;
    
    //!< 账户验证器
    AccountValidator* m_validator;
    
    //!< 交易记录模型
    TransactionModel* m_transactionModel;
}; 