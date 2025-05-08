/**
 * @file OperationResult.cpp
 * @brief 操作结果类实现
 *
 * 实现了OperationResult类中定义的方法。
 */
#include "OperationResult.h"

/**
 * @brief 默认构造函数 - 创建成功结果
 */
OperationResult::OperationResult()
    : success(true)
    , errorMessage(QString())
{
}

/**
 * @brief 带参数构造函数
 * @param success 操作是否成功
 * @param errorMessage 如果失败，存储错误信息
 */
OperationResult::OperationResult(bool success, const QString& errorMessage)
    : success(success)
    , errorMessage(errorMessage)
{
}

/**
 * @brief 创建一个成功的 OperationResult
 * @return 成功的 OperationResult
 */
OperationResult OperationResult::Success()
{
    return OperationResult(true, QString());
}

/**
 * @brief 创建一个失败的 OperationResult
 * @param error 错误信息
 * @return 失败的 OperationResult
 */
OperationResult OperationResult::Failure(const QString &error)
{
    return OperationResult(false, error);
} 