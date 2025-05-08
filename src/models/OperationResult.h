/**
 * @file OperationResult.h
 * @brief 操作结果类
 *
 * 定义了用于表示操作结果的类，包括成功/失败状态和错误信息。
 */
#pragma once

#include <QString>

/**
 * @brief 操作结果结构体
 *
 * 用于表示操作是否成功以及相关的错误信息。
 */
class OperationResult {
public:
    /**
     * @brief 默认构造函数 - 创建成功结果
     */
    OperationResult();
    
    /**
     * @brief 带参数构造函数
     * @param success 操作是否成功
     * @param errorMessage 如果失败，存储错误信息
     */
    OperationResult(bool success, const QString& errorMessage = QString());
    
    /**
     * @brief 操作是否成功
     */
    bool success;
    
    /**
     * @brief 如果失败，存储错误信息
     */
    QString errorMessage;

    /**
     * @brief 创建一个成功的 OperationResult
     * @return 成功的 OperationResult
     */
    static OperationResult Success();
    
    /**
     * @brief 创建一个失败的 OperationResult
     * @param error 错误信息
     * @return 失败的 OperationResult
     */
    static OperationResult Failure(const QString &error);
}; 