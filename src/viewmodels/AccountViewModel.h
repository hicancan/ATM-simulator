// AccountViewModel.h
/**
 * @file AccountViewModel.h
 * @brief 账户视图模型头文件
 *
 * 定义了 AccountViewModel 类，作为 AccountModel 和 UI (QML) 之间的中介。
 * 它暴露账户相关的数据（通过属性）和操作（通过可调用方法）给 QML。
 */
#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>
#include "../models/AccountModel.h"
#include "../models/TransactionModel.h" // 包含 TransactionModel 头文件
#include "../models/AccountValidator.h" // 添加 AccountValidator 头文件

/**
 * @brief 账户视图模型类
 *
 * 提供 AccountModel 到 UI (QML) 的接口。
 * 通过 Q_PROPERTY 暴露账户数据，通过 Q_INVOKABLE 暴露账户操作。
 */
class AccountViewModel : public QObject
{
    Q_OBJECT

    // Q_PROPERTY 宏将属性暴露给 QML
    Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    Q_PROPERTY(QString holderName READ holderName NOTIFY holderNameChanged)
    Q_PROPERTY(double balance READ balance NOTIFY balanceChanged)
    Q_PROPERTY(double predictedBalance READ predictedBalance NOTIFY predictedBalanceChanged)
    Q_PROPERTY(QVariantMap multiDayPredictions READ multiDayPredictions NOTIFY multiDayPredictionsChanged)
    Q_PROPERTY(double withdrawLimit READ withdrawLimit NOTIFY withdrawLimitChanged)
    Q_PROPERTY(bool isLoggedIn READ isLoggedIn NOTIFY isLoggedInChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(bool isAdmin READ isAdmin NOTIFY isAdminChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AccountViewModel(QObject *parent = nullptr);

    /**
     * @brief 设置交易数据模型引用
     *
     * 用于预测余额和记录交易。
     *
     * @param model 交易数据模型指针
     */
    void setTransactionModel(TransactionModel *model);

    // --- 属性获取方法 ---
    QString cardNumber() const;
    QString holderName() const;
    double balance() const;
    double predictedBalance() const;
    QVariantMap multiDayPredictions() const;
    double withdrawLimit() const;
    bool isLoggedIn() const;
    QString errorMessage() const;
    bool isAdmin() const;

    /**
     * @brief 设置当前卡号
     * @param cardNumber 新的卡号
     */
    void setCardNumber(const QString &cardNumber);

    // --- 可调用方法 (供 QML 调用) ---
    /**
     * @brief 处理用户使用 PIN 码登录 (假设卡号已设置)
     * @param pin PIN 码
     * @return 如果登录成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool login(const QString &pin);
    /**
     * @brief 处理用户使用卡号和 PIN 码登录 (用于首次输入卡号)
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 如果登录成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool loginWithCard(const QString &cardNumber, const QString &pin);
    /**
     * @brief 处理管理员登录
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @return 如果登录成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool adminLogin(const QString &cardNumber, const QString &pin);
    /**
     * @brief 处理取款操作
     * @param amount 取款金额
     * @return 如果操作成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool withdraw(double amount);
    /**
     * @brief 处理存款操作
     * @param amount 存款金额
     * @return 如果操作成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool deposit(double amount);
    /**
     * @brief 处理转账操作
     * @param targetCard 目标卡号
     * @param amount 转账金额
     * @return 如果操作成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool transfer(const QString &targetCard, double amount);
    /**
     * @brief 验证转账目标卡号的有效性
     * @param targetCard 目标卡号
     * @return 如果目标卡号有效返回 true，否则返回 false
     */
    Q_INVOKABLE bool validateTargetCard(const QString &targetCard);
    /**
     * @brief 获取转账目标卡号的持卡人姓名 (用于显示)
     * @param targetCard 目标卡号
     * @return 目标卡号的持卡人姓名，如果无效返回空字符串
     */
    Q_INVOKABLE QString getTargetCardHolderName(const QString &targetCard);
    /**
     * @brief 处理修改账户 PIN 码操作
     * @param currentPin 当前 PIN 码
     * @param newPin 新 PIN 码
     * @param confirmPin 确认新 PIN 码
     * @return 如果操作成功返回 true，否则返回 false
     */
    Q_INVOKABLE bool changePassword(const QString &currentPin, const QString &newPin, const QString &confirmPin);
    /**
     * @brief 处理用户登出操作
     */
    Q_INVOKABLE void logout();
    /**
     * @brief 清除当前的错误信息
     */
    Q_INVOKABLE void clearError();
    /**
     * @brief 设置当前的错误信息
     * @param message 错误信息字符串
     */
    Q_INVOKABLE void setErrorMessage(const QString &message);
    /**
     * @brief 计算预测余额
     * @param daysInFuture 预测未来天数 (默认为 7 天)
     */
    Q_INVOKABLE void calculatePredictedBalance(int daysInFuture = 7);
    /**
     * @brief 计算多日期预测余额
     * 预测未来多个时间点的余额变化趋势
     * @param days 预测天数列表，如 "7,14,30,90" 字符串形式
     */
    Q_INVOKABLE void calculateMultiDayPredictions(const QString &days);

    // --- 管理员方法 (可调用) ---
    /**
     * @brief 获取所有账户列表 (管理员权限)
     * @return 包含所有账户数据的 QVariantList
     */
    Q_INVOKABLE QVariantList getAllAccounts();
    /**
     * @brief 创建新账户 (管理员权限)
     * @param cardNumber 卡号
     * @param pin PIN 码
     * @param holderName 持卡人姓名
     * @param balance 初始余额
     * @param withdrawLimit 取款限额
     * @param isLocked 是否锁定
     * @param isAdmin 是否为管理员账户
     * @return 如果成功创建返回 true，否则返回 false
     */
    Q_INVOKABLE bool createAccount(const QString &cardNumber, const QString &pin, const QString &holderName,
                                 double balance, double withdrawLimit, bool isLocked, bool isAdmin);
    /**
     * @brief 更新现有账户信息 (管理员权限)
     * @param cardNumber 卡号
     * @param holderName 持卡人姓名
     * @param balance 余额
     * @param withdrawLimit 取款限额
     * @param isLocked 是否锁定
     * @return 如果成功更新返回 true，否则返回 false
     */
    Q_INVOKABLE bool updateAccount(const QString &cardNumber, const QString &holderName,
                                 double balance, double withdrawLimit, bool isLocked);
    /**
     * @brief 删除账户 (管理员权限)
     * @param cardNumber 要删除的账户卡号
     * @return 如果成功删除返回 true，否则返回 false
     */
    Q_INVOKABLE bool deleteAccount(const QString &cardNumber);
    /**
     * @brief 重置账户 PIN 码 (管理员权限)
     * @param cardNumber 卡号
     * @param newPin 新 PIN 码
     * @return 如果成功重置返回 true，否则返回 false
     */
    Q_INVOKABLE bool resetAccountPin(const QString &cardNumber, const QString &newPin);
    /**
     * @brief 设置账户锁定状态 (管理员权限)
     * @param cardNumber 卡号
     * @param locked 是否锁定
     * @return 如果成功设置返回 true，否则返回 false
     */
    Q_INVOKABLE bool setAccountLockStatus(const QString &cardNumber, bool locked);
    /**
     * @brief 设置账户取款限额 (管理员权限)
     * @param cardNumber 卡号
     * @param limit 新的取款限额
     * @return 如果成功设置返回 true，否则返回 false
     */
    Q_INVOKABLE bool setWithdrawLimit(const QString &cardNumber, double limit);


signals:
    // 通知 QML 属性已改变的信号
    void cardNumberChanged();
    void holderNameChanged();
    void balanceChanged();
    void predictedBalanceChanged();
    void multiDayPredictionsChanged();
    void withdrawLimitChanged();
    void isLoggedInChanged();
    void errorMessageChanged();
    void isAdminChanged();
    /**
     * @brief 用户登出时发出的信号
     */
    void loggedOut();
    /**
     * @brief 交易操作完成时发出的信号
     * @param success 操作是否成功
     * @param message 结果消息 (成功或失败原因)
     */
    void transactionCompleted(bool success, const QString &message);
    /**
     * @brief 账户列表发生变化时发出的信号
     *
     * 例如在创建、更新或删除账户后。
     */
    void accountsChanged();

private:
    //!< AccountModel 实例，处理账户数据和业务逻辑
    AccountModel m_accountModel;
    //!< TransactionModel 指针，用于记录交易和预测余额
    TransactionModel *m_transactionModel;
    //!< AccountValidator 实例，直接处理验证逻辑
    AccountValidator m_validator;

    /**
     * @brief 检查管理员权限
     * @param errorMsg 如果权限检查失败时显示的错误消息
     * @return 如果当前用户有管理员权限返回true，否则返回false
     */
    bool checkAdminPermission(const QString& errorMsg);

    // --- 私有成员变量 (支持 Q_PROPERTY) ---
    QString m_cardNumber;       //!< 当前登录的账户卡号
    QString m_errorMessage;     //!< 当前显示的错误信息
    double m_predictedBalance;  //!< 预测余额
    QVariantMap m_multiDayPredictions; //!< 多日期预测余额
    bool m_isLoggedIn;          //!< 是否已登录
    bool m_isAdmin;             //!< 是否为管理员账户
};