/**
 * @file LoginResult.cpp
 * @brief 登录结果类实现
 *
 * 实现了LoginResult类中定义的方法。
 */
#include "LoginResult.h"

/**
 * @brief 默认构造函数 - 创建失败的登录结果
 */
LoginResult::LoginResult()
    : OperationResult(false, "")
    , isAdmin(false)
    , holderName(QString())
    , balance(0.0)
    , withdrawLimit(0.0)
{
}

/**
 * @brief 带参数构造函数
 * @param success 登录是否成功
 * @param errorMessage 如果失败，存储错误信息
 * @param isAdmin 是否为管理员用户
 * @param holderName 持卡人姓名
 * @param balance 账户余额
 * @param withdrawLimit 取款限额
 */
LoginResult::LoginResult(bool success, const QString& errorMessage,
                        bool isAdmin, const QString& holderName,
                        double balance, double withdrawLimit)
    : OperationResult(success, errorMessage)
    , isAdmin(isAdmin)
    , holderName(holderName)
    , balance(balance)
    , withdrawLimit(withdrawLimit)
{
}

/**
 * @brief 创建一个成功的登录结果
 * @param isAdmin 是否为管理员
 * @param holderName 持卡人姓名
 * @param balance 账户余额
 * @param withdrawLimit 取款限额
 * @return 成功的登录结果
 */
LoginResult LoginResult::Success(bool isAdmin, const QString& holderName,
                                double balance, double withdrawLimit)
{
    return LoginResult(true, QString(), isAdmin, holderName, balance, withdrawLimit);
}

/**
 * @brief 创建一个失败的登录结果
 * @param error 错误信息
 * @return 失败的登录结果
 */
LoginResult LoginResult::Failure(const QString &error)
{
    return LoginResult(false, error);
} 