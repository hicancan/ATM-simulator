// AccountModel.h
/**
 * @file AccountModel.h
 * @brief 账户数据模型门面类
 *
 * 作为统一的门面，整合各个子服务提供完整的账户功能。
 */
#pragma once

#include <QObject>
#include <memory>
#include "IAccountRepository.h"
#include "JsonAccountRepository.h"
#include "AccountValidator.h"
#include "AccountService.h"
#include "AdminService.h"
#include "AccountAnalyticsService.h"
#include "TransactionModel.h"
#include "LoginResult.h"
#include "OperationResult.h"

/**
 * @brief 账户数据模型门面类
 *
 * 作为统一的门面，整合账户相关的多个服务，向上层提供所有账户功能的入口。
 * 设计为轻量级的门面(Facade)模式，委托调用到各个专业服务。
 */
class AccountModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit AccountModel(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~AccountModel();

    /**
     * @brief 设置交易模型
     * @param transactionModel 交易模型指针
     */
    void setTransactionModel(TransactionModel* transactionModel);

    // ================================
    // === AccountService 对应的方法 ===
    // ================================
    
    /**
     * @brief 执行用户登录
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 登录结果
     */
    LoginResult performLogin(const QString &cardNumber, const QString &pin);
    
    /**
     * @brief 执行取款操作
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果
     */
    OperationResult withdrawAmount(const QString &cardNumber, double amount);
    
    /**
     * @brief 执行存款操作
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果
     */
    OperationResult depositAmount(const QString &cardNumber, double amount);
    
    /**
     * @brief 执行转账操作
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果
     */
    OperationResult transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount);
    
    /**
     * @brief 修改PIN码
     * @param cardNumber 卡号
     * @param currentPin 当前PIN码
     * @param newPin 新PIN码
     * @param confirmPin 确认新PIN码
     * @return 操作结果
     */
    OperationResult changePin(const QString &cardNumber, const QString &currentPin, 
                             const QString &newPin, const QString &confirmPin = QString());
    
    /**
     * @brief 获取账户余额
     * @param cardNumber 卡号
     * @return 账户余额
     */
    double getBalance(const QString &cardNumber) const;
    
    /**
     * @brief 获取持卡人姓名
     * @param cardNumber 卡号
     * @return 持卡人姓名
     */
    QString getHolderName(const QString &cardNumber) const;
    
    /**
     * @brief 获取账户取款限额
     * @param cardNumber 卡号
     * @return 取款限额
     */
    double getWithdrawLimit(const QString &cardNumber) const;
    
    /**
     * @brief 检查账户是否被锁定
     * @param cardNumber 卡号
     * @return 如果账户被锁定返回true，否则返回false
     */
    bool isAccountLocked(const QString &cardNumber) const;
    
    // ================================
    // === AdminService 对应的方法 ===
    // ================================
    
    /**
     * @brief 执行管理员登录
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 登录结果
     */
    LoginResult performAdminLogin(const QString &cardNumber, const QString &pin);
    
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
    OperationResult createAccount(const QString &cardNumber, const QString &pin, 
                                 const QString &holderName, double balance, 
                                 double withdrawLimit, bool isAdmin = false);
    
    /**
     * @brief 更新账户信息
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 账户余额
     * @param withdrawLimit 取款限额
     * @param isLocked 是否锁定账户
     * @return 操作结果
     */
    OperationResult updateAccount(const QString &cardNumber, const QString &holderName,
                                 double balance, double withdrawLimit, bool isLocked);
    
    /**
     * @brief 删除账户
     * @param cardNumber 要删除的账户卡号
     * @return 操作结果
     */
    OperationResult deleteAccount(const QString &cardNumber);
    
    /**
     * @brief 设置账户锁定状态
     * @param cardNumber 卡号
     * @param locked 是否锁定
     * @return 操作结果
     */
    OperationResult setAccountLockStatus(const QString &cardNumber, bool locked);
    
    /**
     * @brief 重置PIN码
     * @param cardNumber 卡号
     * @param newPin 新PIN码
     * @return 操作结果
     */
    OperationResult resetPin(const QString &cardNumber, const QString &newPin);
    
    /**
     * @brief 设置取款限额
     * @param cardNumber 卡号
     * @param limit 新限额
     * @return 操作结果
     */
    OperationResult setWithdrawLimit(const QString &cardNumber, double limit);
    
    /**
     * @brief 获取所有账户列表
     * @return 所有账户的列表
     */
    QVector<Account> getAllAccounts() const;
    
    // =========================================
    // === AccountAnalyticsService 对应的方法 ===
    // =========================================
    
    /**
     * @brief 预测未来余额
     * @param cardNumber 卡号
     * @param daysInFuture 预测未来天数
     * @return 预测的余额
     */
    double predictBalance(const QString &cardNumber, int daysInFuture = 7) const;
    
    /**
     * @brief 计算预测余额
     * @param cardNumber 卡号
     * @param daysInFuture 预测未来天数
     * @param outBalance 输出参数，保存预测的余额
     * @return 操作结果
     */
    OperationResult calculatePredictedBalance(const QString &cardNumber,
                                            int daysInFuture,
                                            double &outBalance) const;
    
    /**
     * @brief 获取账户收支趋势
     * @param cardNumber 卡号
     * @param days 分析天数
     * @param outIncomeTrend 输出参数，收入趋势
     * @param outExpenseTrend 输出参数，支出趋势
     * @return 操作结果
     */
    OperationResult getAccountTrend(const QString &cardNumber,
                                   int days,
                                   QMap<QDate, double> &outIncomeTrend,
                                   QMap<QDate, double> &outExpenseTrend) const;
    
    /**
     * @brief 获取交易活跃度
     * @param cardNumber 卡号
     * @param days 分析天数
     * @return 平均每天交易次数
     */
    double getTransactionFrequency(const QString &cardNumber, int days = 30) const;

    /**
     * @brief 验证账户凭据
     * @param cardNumber 卡号
     * @param pin PIN码
     * @return 操作结果
     */
    OperationResult validateCredentials(const QString &cardNumber, const QString &pin) const;

    /**
     * @brief 验证取款操作
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果
     */
    OperationResult validateWithdrawal(const QString &cardNumber, double amount) const;

    /**
     * @brief 验证存款操作
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果
     */
    OperationResult validateDeposit(const QString &cardNumber, double amount) const;

    /**
     * @brief 验证转账操作
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果
     */
    OperationResult validateTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount) const;

    /**
     * @brief 记录交易
     * @param cardNumber 交易涉及的卡号
     * @param type 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param description 交易描述
     * @param targetCard 目标卡号 (转账时使用)
     */
    void recordTransaction(const QString &cardNumber, 
                          TransactionType type,
                          double amount, 
                          double balanceAfter,
                          const QString &description, 
                          const QString &targetCard = QString());

    /**
     * @brief 验证目标账户
     * @param targetCardNumber 目标卡号
     * @return 操作结果
     */
    OperationResult validateTargetAccount(const QString &targetCardNumber) const;

    /**
     * @brief 获取目标账户信息
     * @param targetCardNumber 目标卡号
     * @param outHolderName 输出参数，目标持卡人姓名
     * @param outIsLocked 输出参数，目标账户是否锁定
     * @return 如果成功获取返回true，否则返回false
     */
    bool getTargetAccountInfo(const QString &targetCardNumber, 
                             QString &outHolderName, 
                             bool &outIsLocked) const;

    /**
     * @brief 将所有账户转换为变体列表（用于UI显示）
     * @return 包含账户数据的QVariantList
     */
    QVariantList getAllAccountsAsVariantList() const;

    /**
     * @brief 验证创建账户参数
     * @param cardNumber 卡号
     * @param pin PIN码
     * @param holderName 持卡人姓名
     * @param balance 初始余额
     * @param withdrawLimit 取款限额
     * @param isAdmin 是否为管理员账户
     * @return 操作结果
     */
    OperationResult validateCreateAccount(const QString &cardNumber, 
                                        const QString &pin, 
                                        const QString &holderName,
                                        double balance, 
                                        double withdrawLimit, 
                                        bool isAdmin = false) const;

    /**
     * @brief 从视图模型更新账户
     * @param account 账户对象
     * @return 操作结果
     */
    OperationResult updateAccountFromViewModel(const Account &account);

    /**
     * @brief 验证管理员操作
     * @param adminCardNumber 管理员卡号
     * @return 操作结果
     */
    OperationResult validateAdminOperation(const QString &adminCardNumber) const;

    /**
     * @brief 检查账户是否存在
     * @param cardNumber 卡号
     * @return 如果账户存在返回true，否则返回false
     */
    bool accountExists(const QString &cardNumber) const;

private:
    //!< 账户存储库
    std::unique_ptr<IAccountRepository> m_repository;
    
    //!< 账户验证器
    std::unique_ptr<AccountValidator> m_validator;
    
    //!< 账户服务
    std::unique_ptr<AccountService> m_accountService;
    
    //!< 管理员服务
    std::unique_ptr<AdminService> m_adminService;
    
    //!< 账户分析服务
    std::unique_ptr<AccountAnalyticsService> m_analyticsService;
    
    //!< 交易记录模型
    TransactionModel* m_transactionModel;
};