/**
 * @file IAccountRepository.h
 * @brief 账户存储库接口
 *
 * 定义了账户数据访问的抽象接口，支持不同的后端存储实现。
 */
#pragma once

#include <QString>
#include <QVector>
#include <optional>
#include "Account.h"
#include "OperationResult.h"

/**
 * @brief 账户存储库接口
 *
 * 定义了账户数据访问的抽象接口，用于不同的存储实现（如JSON文件、数据库等）。
 */
class IAccountRepository {
public:
    /**
     * @brief 虚析构函数
     */
    virtual ~IAccountRepository() = default;
    
    /**
     * @brief 保存单个账户
     * @param account 要保存的账户
     * @return 操作结果
     */
    virtual OperationResult saveAccount(const Account& account) = 0;
    
    /**
     * @brief 删除账户
     * @param cardNumber 要删除的账户卡号
     * @return 操作结果
     */
    virtual OperationResult deleteAccount(const QString& cardNumber) = 0;
    
    /**
     * @brief 根据卡号查找账户
     * @param cardNumber 卡号
     * @return 包含账户的optional对象，如果未找到则为empty
     */
    virtual std::optional<Account> findByCardNumber(const QString& cardNumber) const = 0;
    
    /**
     * @brief 获取所有账户
     * @return 所有账户的列表
     */
    virtual QVector<Account> getAllAccounts() const = 0;
    
    /**
     * @brief 保存所有账户数据
     * @return 如果成功保存返回true，否则返回false
     */
    virtual bool saveAccounts() = 0;
    
    /**
     * @brief 加载所有账户数据
     * @return 如果成功加载返回true，否则返回false
     */
    virtual bool loadAccounts() = 0;
    
    /**
     * @brief 检查账户是否存在
     * @param cardNumber 卡号
     * @return 如果账户存在返回true，否则返回false
     */
    virtual bool accountExists(const QString& cardNumber) const = 0;
}; 