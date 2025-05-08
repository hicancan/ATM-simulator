// JsonAccountRepository.h
/**
 * @file JsonAccountRepository.h
 * @brief JSON账户存储库类
 *
 * 使用JSON文件作为后端存储实现IAccountRepository接口。
 */
#pragma once

#include <QMap>
#include <QString>
#include <QVector>
#include <optional>
#include "IAccountRepository.h"
#include "Account.h"

/**
 * @brief JSON账户存储库类
 *
 * 使用JSON文件存储实现账户数据访问。
 */
class JsonAccountRepository : public IAccountRepository {
public:
    /**
     * @brief 构造函数
     * @param dataPath 数据存储路径，默认使用应用程序的本地数据目录
     * @param filename 账户数据文件名，默认为"accounts.json"
     */
    explicit JsonAccountRepository(const QString& dataPath = QString(), 
                                  const QString& filename = "accounts.json");
    
    /**
     * @brief 析构函数
     *
     * 在对象销毁时保存账户数据。
     */
    ~JsonAccountRepository() override;
    
    /**
     * @brief 保存单个账户
     * @param account 要保存的账户
     * @return 操作结果
     */
    OperationResult saveAccount(const Account& account) override;
    
    /**
     * @brief 删除账户
     * @param cardNumber 要删除的账户卡号
     * @return 操作结果
     */
    OperationResult deleteAccount(const QString& cardNumber) override;
    
    /**
     * @brief 根据卡号查找账户
     * @param cardNumber 卡号
     * @return 包含账户的optional对象，如果未找到则为empty
     */
    std::optional<Account> findByCardNumber(const QString& cardNumber) const override;
    
    /**
     * @brief 获取所有账户
     * @return 所有账户的列表
     */
    QVector<Account> getAllAccounts() const override;
    
    /**
     * @brief 保存所有账户数据
     * @return 如果成功保存返回true，否则返回false
     */
    bool saveAccounts() override;
    
    /**
     * @brief 加载所有账户数据
     * @return 如果成功加载返回true，否则返回false
     */
    bool loadAccounts() override;
    
    /**
     * @brief 检查账户是否存在
     * @param cardNumber 卡号
     * @return 如果账户存在返回true，否则返回false
     */
    bool accountExists(const QString& cardNumber) const override;

private:
    /**
     * @brief 初始化测试账户数据
     *
     * 如果数据文件不存在，则创建一些预设的测试账户。
     */
    void initializeTestAccounts();
    
    /**
     * @brief 添加账户到内存映射
     * @param account 要添加的账户
     */
    void addAccount(const Account& account);
    
    //!< 账户内存存储（卡号->账户映射）
    QMap<QString, Account> m_accounts;
    
    //!< 数据存储路径
    QString m_dataPath;
    
    //!< 账户数据文件名
    QString m_filename;
}; 