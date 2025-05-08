// AccountModel.h
/**
 * @file AccountModel.h
 * @brief 账户数据模型头文件
 *
 * 定义了账户结构体、操作结果结构体以及账户数据管理类 AccountModel。
 * AccountModel 负责账户数据的加载、保存、验证和执行各种账户操作，
 * 如登录、取款、存款、转账、修改 PIN 等，并支持管理员功能。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVector>
#include <QMap>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantMap>
#include "TransactionModel.h"

/**
 * @brief 账户数据结构体
 *
 * 存储单个账户的详细信息。
 */
struct Account {
    QString cardNumber;     //!< 卡号，唯一标识账户
    QString pin;            //!< PIN 码，用于身份验证
    QString holderName;     //!< 持卡人姓名
    double balance;         //!< 当前账户余额
    double withdrawLimit;   //!< 单次取款限额
    bool isLocked;          //!< 账户是否被锁定
    bool isAdmin;           //!< 是否为管理员账户

    /**
     * @brief 将 Account 对象转换为 QJsonObject
     * @return 包含账户数据的 QJsonObject
     */
    QJsonObject toJson() const {
        QJsonObject json;
        json["cardNumber"] = cardNumber;
        json["pin"] = pin;
        json["holderName"] = holderName;
        json["balance"] = balance;
        json["withdrawLimit"] = withdrawLimit;
        json["isLocked"] = isLocked;
        json["isAdmin"] = isAdmin;
        return json;
    }

    /**
     * @brief 从 QJsonObject 创建 Account 对象
     * @param json 包含账户数据的 QJsonObject
     * @return 创建的 Account 对象
     */
    static Account fromJson(const QJsonObject &json) {
        Account account;
        account.cardNumber = json["cardNumber"].toString();
        account.pin = json["pin"].toString();
        account.holderName = json["holderName"].toString();
        account.balance = json["balance"].toDouble();
        account.withdrawLimit = json["withdrawLimit"].toDouble();
        account.isLocked = json["isLocked"].toBool();
        // 兼容旧数据，如果 isAdmin 字段不存在，默认为 false
        account.isAdmin = json.contains("isAdmin") ? json["isAdmin"].toBool() : false;
        return account;
    }
};

/**
 * @brief 操作结果结构体
 *
 * 用于表示操作是否成功以及相关的错误信息。
 */
struct OperationResult {
    bool success;           //!< 操作是否成功
    QString errorMessage;   //!< 如果失败，存储错误信息

    /**
     * @brief 创建一个成功的 OperationResult
     * @return 成功的 OperationResult
     */
    static OperationResult Success() { return {true, ""}; }
    /**
     * @brief 创建一个失败的 OperationResult
     * @param error 错误信息
     * @return 失败的 OperationResult
     */
    static OperationResult Failure(const QString &error) { return {false, error}; }
};

/**
 * @brief 登录结果结构体
 *
 * 用于表示登录操作的结果及相关的账户信息。
 */
struct LoginResult {
    bool success;           //!< 登录是否成功
    QString errorMessage;   //!< 如果失败，存储错误信息
    bool isAdmin;           //!< 是否为管理员用户
    QString holderName;     //!< 持卡人姓名
    double balance;         //!< 账户余额
    double withdrawLimit;   //!< 取款限额
};

/**
 * @brief 账户数据模型类
 *
 * 负责管理账户数据，包括数据的加载、保存、验证和业务逻辑处理。
 * 不直接与 UI 交互，通过属性和方法向 ViewModel 提供数据和功能。
 */
class AccountModel : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AccountModel(QObject *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~AccountModel();

    /**
     * @brief 设置交易数据模型引用
     *
     * 用于在账户操作时记录交易。
     *
     * @param transactionModel 交易数据模型指针
     */
    void setTransactionModel(TransactionModel* transactionModel);

    /**
     * @brief 记录交易
     *
     * 调用关联的 TransactionModel 记录交易信息。
     *
     * @param cardNumber 交易涉及的卡号
     * @param type 交易类型
     * @param amount 交易金额
     * @param balanceAfter 交易后余额
     * @param description 交易描述
     * @param targetCard 目标卡号 (转账时使用)
     */
    void recordTransaction(const QString &cardNumber, TransactionType type,
                           double amount, double balanceAfter,
                           const QString &description, const QString &targetCard = QString());

    /**
     * @brief 执行取款操作
     * 
     * 验证取款操作并执行账户余额的扣除。
     *
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult withdrawAmount(const QString &cardNumber, double amount);

    /**
     * @brief 执行存款操作
     * 
     * 验证存款操作并执行账户余额的增加。
     *
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult depositAmount(const QString &cardNumber, double amount);

    /**
     * @brief 执行转账操作
     * 
     * 验证转账操作并执行源账户余额的扣除和目标账户余额的增加。
     *
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult transferAmount(const QString &fromCardNumber, const QString &toCardNumber, double amount);

    /**
     * @brief 修改账户 PIN 码
     * 
     * 验证PIN码修改操作并更新账户的PIN码。
     *
     * @param cardNumber 卡号
     * @param currentPin 当前 PIN 码
     * @param newPin 新 PIN 码
     * @param confirmPin 确认新 PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult changePin(const QString &cardNumber, const QString &currentPin, 
                              const QString &newPin, const QString &confirmPin = QString());

    /**
     * @brief 执行完整的登录流程
     *
     * 验证凭据并返回登录结果及相关账户信息。
     *
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 登录结果结构体
     */
    LoginResult performLogin(const QString &cardNumber, const QString &pin);
    
    /**
     * @brief 执行完整的管理员登录流程
     *
     * 验证管理员凭据并返回登录结果。
     *
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 登录结果结构体
     */
    LoginResult performAdminLogin(const QString &cardNumber, const QString &pin);

    // --- 验证方法 ---
    /**
     * @brief 验证账户凭据
     *
     * 检查卡号和 PIN 是否匹配且账户未被锁定。
     *
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateCredentials(const QString &cardNumber, const QString &pin);

    /**
     * @brief 验证登录操作
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateLogin(const QString &cardNumber, const QString &pin);

    /**
     * @brief 验证管理员登录
     * @param cardNumber 管理员卡号
     * @param pin 管理员 PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateAdminLogin(const QString &cardNumber, const QString &pin);

    /**
     * @brief 验证取款操作
     *
     * 检查账户是否存在、未锁定、余额充足且未超限额。
     *
     * @param cardNumber 卡号
     * @param amount 取款金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateWithdrawal(const QString &cardNumber, double amount);
    
    /**
     * @brief 验证存款操作
     *
     * 检查账户是否存在、未锁定且金额有效。
     *
     * @param cardNumber 卡号
     * @param amount 存款金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateDeposit(const QString &cardNumber, double amount);
    
    /**
     * @brief 验证转账操作
     *
     * 检查源账户和目标账户是否存在、源账户未锁定、目标账户未锁定、金额有效且余额充足。
     *
     * @param fromCardNumber 源卡号
     * @param toCardNumber 目标卡号
     * @param amount 转账金额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateTransfer(const QString &fromCardNumber, const QString &toCardNumber, double amount);
    
    /**
     * @brief 验证 PIN 码修改操作
     *
     * 检查账户是否存在、未锁定、当前 PIN 码正确且新 PIN 码格式和确认 PIN 码匹配。
     *
     * @param cardNumber 卡号
     * @param currentPin 当前 PIN 码
     * @param newPin 新 PIN 码
     * @param confirmPin 确认新 PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validatePinChange(const QString &cardNumber, const QString &currentPin, 
                                     const QString &newPin, const QString &confirmPin);

    /**
     * @brief 验证目标账户（转账时使用）
     *
     * 检查源账户和目标账户是否存在，且目标账户未被锁定。
     *
     * @param sourceCard 源卡号
     * @param targetCard 目标卡号
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateTargetAccount(const QString &sourceCard, const QString &targetCard);
    
    /**
     * @brief 验证管理员操作权限
     *
     * 检查给定卡号是否为管理员账户。
     *
     * @param cardNumber 卡号
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateAdminOperation(const QString &cardNumber);

    /**
     * @brief 验证创建账户参数
     *
     * 检查卡号格式、是否存在冲突、PIN 码格式、持卡人姓名、余额和限额是否有效。
     *
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @param holderName 持卡人姓名
     * @param balance 初始余额
     * @param withdrawLimit 取款限额
     * @param isAdmin 是否为管理员账户
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateCreateAccount(const QString &cardNumber, const QString &pin, const QString &holderName,
                                      double balance, double withdrawLimit, bool isAdmin);
                                      
    /**
     * @brief 验证更新账户参数
     *
     * 检查账户是否存在、持卡人姓名、余额和限额是否有效。
     *
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 余额
     * @param withdrawLimit 取款限额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult validateUpdateAccount(const QString &cardNumber, const QString &holderName,
                                      double balance, double withdrawLimit);

    // --- 数据访问方法 ---
    /**
     * @brief 检查账户是否存在
     * @param cardNumber 卡号
     * @return 如果账户存在返回 true，否则返回 false
     */
    bool accountExists(const QString &cardNumber) const;
    
    /**
     * @brief 获取账户持卡人姓名
     * @param cardNumber 卡号
     * @return 持卡人姓名，如果账户不存在返回空字符串
     */
    QString getHolderName(const QString &cardNumber) const;
    
    /**
     * @brief 获取账户余额
     * @param cardNumber 卡号
     * @return 账户余额，如果账户不存在返回 0.0
     */
    double getBalance(const QString &cardNumber) const;
    
    /**
     * @brief 获取账户取款限额
     * @param cardNumber 卡号
     * @return 取款限额，如果账户不存在返回 0.0
     */
    double getWithdrawLimit(const QString &cardNumber) const;
    
    /**
     * @brief 检查账户是否被锁定
     * @param cardNumber 卡号
     * @return 如果账户被锁定返回 true，否则返回 false
     */
    bool isAccountLocked(const QString &cardNumber) const;
    
    /**
     * @brief 检查账户是否为管理员账户
     * @param cardNumber 卡号
     * @return 如果是管理员账户返回 true，否则返回 false
     */
    bool isAdmin(const QString &cardNumber) const;
    
    /**
     * @brief 获取目标账户信息（转账时用于显示）
     *
     * 验证目标账户是否存在并返回其持卡人姓名。
     *
     * @param cardNumber 当前用户卡号
     * @param targetCardNumber 目标卡号
     * @return 目标账户持卡人姓名，如果账户不存在或验证失败返回空字符串
     */
    QString getTargetAccountInfo(const QString &cardNumber, const QString &targetCardNumber);
    
    /**
     * @brief 获取所有账户列表
     * @return 包含所有 Account 结构体的 QVector
     */
    QVector<Account> getAllAccounts() const;

    // --- 账户管理方法 (管理员功能) ---
    /**
     * @brief 创建账户
     * @param account 账户数据结构
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult createAccount(const Account &account);
    
    /**
     * @brief 更新账户
     * @param account 账户数据结构
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult updateAccount(const Account &account);
    
    /**
     * @brief 删除账户
     * @param cardNumber 卡号
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult deleteAccount(const QString &cardNumber);
    
    /**
     * @brief 设置账户锁定状态
     * @param cardNumber 卡号
     * @param locked 是否锁定
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult setAccountLockStatus(const QString &cardNumber, bool locked);
    
    /**
     * @brief 设置账户取款限额
     * @param cardNumber 卡号
     * @param limit 取款限额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult setWithdrawLimit(const QString &cardNumber, double limit);
    
    /**
     * @brief 重置账户 PIN 码 (管理员功能)
     *
     * 为指定账户设置新的 PIN 码。
     *
     * @param cardNumber 卡号
     * @param newPin 新 PIN 码
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult resetPin(const QString &cardNumber, const QString &newPin);
    
    /**
     * @brief 从 ViewModel 更新账户信息
     *
     * 更新账户的可编辑字段（持卡人姓名、余额、限额、锁定状态），保留 PIN 和管理员状态。
     *
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 余额
     * @param withdrawLimit 取款限额
     * @param isLocked 是否锁定
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult updateAccountFromViewModel(const QString &cardNumber, const QString &holderName,
                                           double balance, double withdrawLimit, bool isLocked);

    // --- 预测方法 ---
    /**
     * @brief 预测账户余额
     *
     * 根据历史交易记录预测未来一定天数后的余额。
     *
     * @param cardNumber 卡号
     * @param transactionModel 交易数据模型指针
     * @param daysInFuture 预测未来天数
     * @return 预测的余额，如果无法预测返回当前余额
     */
    double predictBalance(const QString &cardNumber, const TransactionModel* transactionModel, int daysInFuture = 7) const;
    
    /**
     * @brief 计算预测余额（包含验证逻辑）
     *
     * 验证输入参数并调用预测余额方法。
     *
     * @param cardNumber 卡号
     * @param transactionModel 交易数据模型指针
     * @param daysInFuture 预测未来天数
     * @param outBalance 输出参数，存储计算出的预测余额
     * @return 操作结果 (成功或失败及错误信息)
     */
    OperationResult calculatePredictedBalance(const QString &cardNumber,
                                            const TransactionModel* transactionModel,
                                            int daysInFuture,
                                            double &outBalance);

    // --- 持久化存储方法 ---
    /**
     * @brief 保存账户数据到文件
     * @param filename 文件名 (默认为 accounts.json)
     * @return 如果成功保存返回 true，否则返回 false
     */
    bool saveAccounts(const QString &filename = "accounts.json");
    
    /**
     * @brief 从文件加载账户数据
     * @param filename 文件名 (默认为 accounts.json)
     * @return 如果成功加载返回 true，否则返回 false
     */
    bool loadAccounts(const QString &filename = "accounts.json");


    // --- 辅助方法 ---
    /**
     * @brief 添加账户到内存列表
     * @param account 要添加的 Account 对象
     */
    void addAccount(const Account &account);
    
    /**
     * @brief 初始化测试账户数据
     *
     * 如果数据文件不存在，则创建一些预设的测试账户。
     */
    void initializeTestAccounts();
    
    /**
     * @brief 将 Account 对象转换为 QVariantMap
     *
     * 方便在 QML 中使用。
     *
     * @param account 要转换的 Account 对象
     * @return 对应的 QVariantMap
     */
    QVariantMap accountToVariantMap(const Account &account) const;
    
    /**
     * @brief 获取所有账户并转换为 QVariantList
     *
     * 方便在 QML 的 ListView 等控件中显示。
     *
     * @return 包含所有账户数据的 QVariantList
     */
    QVariantList getAllAccountsAsVariantList() const;


private:
    //!< 账户数据存储（使用 QMap 模拟数据库）
    QMap<QString, Account> m_accounts;

    //!< 交易数据模型指针
    TransactionModel* m_transactionModel;

    /**
     * @brief 根据卡号查找账户（可修改版本）
     * @param cardNumber 卡号
     * @return 账户指针，如果未找到返回 nullptr
     */
    Account* findAccount(const QString &cardNumber);
    
    /**
     * @brief 根据卡号查找账户（常量版本）
     * @param cardNumber 卡号
     * @return 账户指针，如果未找到返回 nullptr
     */
    const Account* findAccount(const QString &cardNumber) const;

    //!< 数据存储路径
    QString m_dataPath;
};