/**
 * @file AccountValidator.h
 * @brief 账户验证器类
 *
 * 集中处理所有与账户操作相关的验证逻辑。
 */
#pragma once

#include <QString>
#include <functional>
#include <vector>
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
    
    /**
     * @brief 验证账户是否存在
     * @param cardNumber 卡号
     * @return 操作结果，如果账户不存在则返回失败
     */
    OperationResult validateAccountExists(const QString& cardNumber) const;
    
    /**
     * @brief 验证账户是否锁定
     * @param cardNumber 卡号
     * @return 操作结果，如果账户锁定则返回失败
     */
    OperationResult validateAccountNotLocked(const QString& cardNumber) const;
    
    /**
     * @brief 验证账户余额是否足够
     * @param cardNumber 卡号
     * @param amount 需要的金额
     * @return 操作结果，如果余额不足则返回失败
     */
    OperationResult validateSufficientBalance(const QString& cardNumber, double amount) const;
    
    /**
     * @brief 验证取款限额
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果，如果超过限额则返回失败
     */
    OperationResult validateWithdrawLimit(const QString& cardNumber, double amount) const;
    
    /**
     * @brief 验证金额是否为100的倍数
     * @param amount 金额
     * @param operationType 操作类型(如"取款"、"存款")
     * @return 操作结果，如果金额不是100的倍数则返回失败
     */
    OperationResult validateAmountMultipleOf100(double amount, const QString& operationType) const;
    
    // 新增方法 - ViewModel层专用的验证接口
    OperationResult validateLoginInput(const QString& cardNumber, const QString& pin) const;
    OperationResult validateWithdrawalInput(const QString& cardNumber, double amount) const;
    OperationResult validateDepositInput(const QString& cardNumber, double amount) const;
    OperationResult validateTransferInput(const QString& cardNumber, const QString& targetCard, double amount) const;
    OperationResult validatePinChangeInput(const QString& cardNumber, const QString& currentPin, 
                                    const QString& newPin, const QString& confirmPin) const;
    
    // 通用方法 - 检查是否已登录
    OperationResult validateLoggedInStatus(bool isLoggedIn, const QString& cardNumber = QString()) const;
    
private:
    // 定义验证函数类型
    using ValidationFunction = std::function<OperationResult()>;
    
    /**
     * @brief 通用验证方法，按顺序执行多个验证步骤
     * @param validations 验证函数列表
     * @return 第一个失败的验证结果，或者全部成功时返回成功结果
     */
    OperationResult validateOperation(const std::vector<ValidationFunction>& validations) const;
    
    //!< 账户存储库接口指针
    IAccountRepository* m_repository;
}; 