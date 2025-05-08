/**
 * @file LoginResult.h
 * @brief 登录结果类
 *
 * 定义了用于表示登录操作结果的类，扩展了OperationResult，包含用户信息。
 */
#pragma once

#include <QString>
#include "OperationResult.h"

/**
 * @brief 登录结果类
 *
 * 用于表示登录操作的结果及相关的账户信息。
 * 继承自OperationResult，添加了账户特定信息。
 */
class LoginResult : public OperationResult {
public:
    /**
     * @brief 默认构造函数 - 创建失败的登录结果
     */
    LoginResult();
    
    /**
     * @brief 带参数构造函数
     * @param success 登录是否成功
     * @param errorMessage 如果失败，存储错误信息
     * @param isAdmin 是否为管理员用户
     * @param holderName 持卡人姓名
     * @param balance 账户余额
     * @param withdrawLimit 取款限额
     */
    LoginResult(bool success, const QString& errorMessage,
               bool isAdmin = false, const QString& holderName = QString(),
               double balance = 0.0, double withdrawLimit = 0.0);
    
    /**
     * @brief 是否为管理员用户
     */
    bool isAdmin;
    
    /**
     * @brief 持卡人姓名
     */
    QString holderName;
    
    /**
     * @brief 账户余额
     */
    double balance;
    
    /**
     * @brief 取款限额
     */
    double withdrawLimit;
    
    /**
     * @brief 创建一个成功的登录结果
     * @param isAdmin 是否为管理员
     * @param holderName 持卡人姓名
     * @param balance 账户余额
     * @param withdrawLimit 取款限额
     * @return 成功的登录结果
     */
    static LoginResult Success(bool isAdmin, const QString& holderName,
                              double balance, double withdrawLimit);
    
    /**
     * @brief 创建一个失败的登录结果
     * @param error 错误信息
     * @return 失败的登录结果
     */
    static LoginResult Failure(const QString &error);
}; 