// AccountViewModel.cpp
/**
 * @file AccountViewModel.cpp
 * @brief 账户视图模型实现文件
 *
 * 实现了 AccountViewModel 类中定义的属性获取、设置和可调用方法。
 * 作为 AccountModel 和 UI (QML) 之间的中介，处理用户输入，
 * 调用 Model 层的业务逻辑，并更新 UI 状态。
 */
#include "AccountViewModel.h"
#include <QDateTime>
#include <QDebug>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
AccountViewModel::AccountViewModel(QObject *parent)
    : QObject(parent)
    , m_transactionModel(nullptr) //!< 初始化交易模型指针为空
    , m_validator(m_accountModel.getRepository()) // 初始化验证器，使用AccountModel中的repository
    , m_isLoggedIn(false)
    , m_predictedBalance(0.0)
    , m_isAdmin(false)
    , m_cardNumber("")
    , m_errorMessage("")
    , m_balance(0.0)
    , m_withdrawLimit(0.0)
    , m_holderName("")
    , m_multiDayPredictions()
{
    // 构造函数初始化成员变量，无复杂逻辑。
}

/**
 * @brief 设置交易数据模型引用
 * @param model 交易数据模型指针
 */
void AccountViewModel::setTransactionModel(TransactionModel *model)
{
    m_transactionModel = model;
    // 重要：确保也将 TransactionModel 传递给 AccountModel，
    // AccountModel 在记录交易和预测余额时需要使用它。
    m_accountModel.setTransactionModel(model);
}

// --- 属性获取方法 ---

/**
 * @brief 获取当前卡号
 * @return 当前卡号
 */
QString AccountViewModel::cardNumber() const
{
    return m_cardNumber;
}

/**
 * @brief 设置当前卡号
 * @param cardNumber 新的卡号
 */
void AccountViewModel::setCardNumber(const QString &cardNumber)
{
    if (m_cardNumber != cardNumber) {
        m_cardNumber = cardNumber;
        emit cardNumberChanged(); // 通知 QML 属性改变

        // 当卡号改变时，重置预测余额
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
    }
}

/**
 * @brief 获取持卡人姓名
 * @return 如果已登录，返回持卡人姓名；否则返回空字符串
 */
QString AccountViewModel::holderName() const
{
    // 只在登录状态下返回持卡人姓名
    return m_isLoggedIn ? m_accountModel.getHolderName(m_cardNumber) : QString();
}

/**
 * @brief 获取账户余额
 * @return 如果已登录，返回账户余额；否则返回 0.0
 */
double AccountViewModel::balance() const
{
    // 只在登录状态下返回余额
    return m_isLoggedIn ? m_accountModel.getBalance(m_cardNumber) : 0.0;
}

/**
 * @brief 获取取款限额
 * @return 如果已登录，返回取款限额；否则返回 0.0
 */
double AccountViewModel::withdrawLimit() const
{
    // 只在登录状态下返回取款限额
    return m_isLoggedIn ? m_accountModel.getWithdrawLimit(m_cardNumber) : 0.0;
}

/**
 * @brief 获取登录状态
 * @return 如果已登录返回 true，否则返回 false
 */
bool AccountViewModel::isLoggedIn() const
{
    return m_isLoggedIn;
}

/**
 * @brief 获取当前的错误信息
 * @return 错误信息字符串
 */
QString AccountViewModel::errorMessage() const
{
    return m_errorMessage;
}

/**
 * @brief 获取管理员状态
 * @return 如果当前登录用户是管理员返回 true，否则返回 false
 */
bool AccountViewModel::isAdmin() const
{
    return m_isAdmin;
}

/**
 * @brief 获取预测余额属性
 * @return 预测余额
 */
double AccountViewModel::predictedBalance() const
{
    return m_predictedBalance;
}

/**
 * @brief 获取多日期预测余额属性
 * @return 保存多个预测日期及对应余额的映射
 */
QVariantMap AccountViewModel::multiDayPredictions() const
{
    return m_multiDayPredictions;
}

// --- 可调用方法 (供 QML 调用) ---

/**
 * @brief 处理用户使用 PIN 码登录 (假设卡号已设置)
 * @param pin PIN 码
 * @return 如果登录成功返回 true，否则返回 false
 */
bool AccountViewModel::login(const QString &pin)
{
    clearError();

    // 验证卡号是否已设置
    if (m_cardNumber.isEmpty()) {
        setErrorMessage("请先输入卡号");
        return false;
    }

    // 调用 Model 层验证账户凭据
    OperationResult result = m_validator.validateCredentials(m_cardNumber, pin);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 登录成功，执行登录过程获取账户详情
    LoginResult loginResult = m_accountModel.performLogin(m_cardNumber, pin);
    if (loginResult.success) {
        m_isLoggedIn = true;
        m_isAdmin = loginResult.isAdmin;
        
        // 移除登录交易记录
        // recordTransaction(TransactionType::Other, 0.0, loginResult.balance, "登录系统");

        // 发出信号通知 UI
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        emit isAdminChanged();
        
        qDebug() << "成功登录系统，卡号:" << m_cardNumber << "，管理员权限:" << m_isAdmin;
        return true;
    } else {
        setErrorMessage(loginResult.errorMessage);
        return false;
    }
}

/**
 * @brief 处理用户使用卡号和 PIN 码登录 (用于首次输入卡号)
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @return 如果登录成功返回 true，否则返回 false
 */
bool AccountViewModel::loginWithCard(const QString &cardNumber, const QString &pin)
{
    // 先设置卡号
    setCardNumber(cardNumber);
    
    // 然后使用通用登录方法
    return login(pin);
}

/**
 * @brief 处理管理员登录
 * @param cardNumber 卡号
 * @param pin PIN 码
 * @return 如果登录成功返回 true，否则返回 false
 */
bool AccountViewModel::adminLogin(const QString &cardNumber, const QString &pin)
{
    clearError();
    
    // 设置卡号
    setCardNumber(cardNumber);
    
    // 调用管理员登录方法
    LoginResult loginResult = m_accountModel.performAdminLogin(cardNumber, pin);
    if (loginResult.success) {
        m_isLoggedIn = true;
        m_isAdmin = true; // 管理员登录，强制设置为管理员
        
        // 发出信号通知 UI
        emit isLoggedInChanged();
        emit holderNameChanged();
        emit balanceChanged();
        emit withdrawLimitChanged();
        emit isAdminChanged();
        
        qDebug() << "管理员成功登录系统，卡号:" << m_cardNumber;
        return true;
    } else {
        setErrorMessage(loginResult.errorMessage);
        return false;
    }
}

/**
 * @brief 处理取款操作
 * @param amount 取款金额
 * @return 如果操作成功返回 true，否则返回 false
 */
bool AccountViewModel::withdraw(double amount)
{
    clearError();

    // 首先检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 验证取款参数是否合法
    OperationResult result = m_validator.validateWithdrawal(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 执行取款操作
    OperationResult withdrawResult = m_accountModel.withdrawAmount(m_cardNumber, amount);
    if (withdrawResult.success) {
        // 不再需要在此记录交易，AccountService 已经记录了
        // 移除冗余代码:
        // recordTransaction(TransactionType::Withdrawal, amount, 
        //                  m_accountModel.getBalance(m_cardNumber), 
        //                  "取款");
        
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        emit transactionCompleted(true, QString("成功取款 %1 元").arg(amount));
        return true;
    } else {
        setErrorMessage(withdrawResult.errorMessage);
        emit transactionCompleted(false, withdrawResult.errorMessage);
        return false;
    }
}

/**
 * @brief 处理存款操作
 * @param amount 存款金额
 * @return 如果操作成功返回 true，否则返回 false
 */
bool AccountViewModel::deposit(double amount)
{
    clearError();

    // 首先检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 验证存款参数是否合法
    OperationResult result = m_validator.validateDeposit(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 执行存款操作
    OperationResult depositResult = m_accountModel.depositAmount(m_cardNumber, amount);
    if (depositResult.success) {
        // 不再需要在此记录交易，AccountService 已经记录了
        // 移除冗余代码:
        // recordTransaction(TransactionType::Deposit, amount, 
        //                  m_accountModel.getBalance(m_cardNumber), 
        //                  "存款");
        
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        emit transactionCompleted(true, QString("成功存款 %1 元").arg(amount));
        return true;
    } else {
        setErrorMessage(depositResult.errorMessage);
        emit transactionCompleted(false, depositResult.errorMessage);
        return false;
    }
}

/**
 * @brief 处理转账操作
 * @param targetCard 目标卡号
 * @param amount 转账金额
 * @return 如果操作成功返回 true，否则返回 false
 */
bool AccountViewModel::transfer(const QString &targetCard, double amount)
{
    clearError();

    // 首先检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 验证转账参数是否合法
    OperationResult result = m_validator.validateTransfer(m_cardNumber, targetCard, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 执行转账操作
    OperationResult transferResult = m_accountModel.transferAmount(m_cardNumber, targetCard, amount);
    if (transferResult.success) {
        // 不再需要在这里记录交易，因为 AccountService 已经记录了
        // 移除以下代码块，避免重复记录:
        // String targetHolderName = getTargetCardHolderName(targetCard);
        // String description = QString("转账 %1 元到 %2").arg(amount).arg(targetHolderName.isEmpty() ? targetCard : targetHolderName);
        // recordTransaction(TransactionType::Transfer, amount, 
        //                  m_accountModel.getBalance(m_cardNumber), 
        //                  description, targetCard);
        
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        emit transactionCompleted(true, QString("成功转账 %1 元到账户 %2").arg(amount).arg(targetCard));
        return true;
    } else {
        setErrorMessage(transferResult.errorMessage);
        emit transactionCompleted(false, transferResult.errorMessage);
        return false;
    }
}

/**
 * @brief 验证转账目标卡号的有效性
 * @param targetCard 目标卡号
 * @return 如果目标卡号有效返回 true，否则返回 false
 */
bool AccountViewModel::validateTargetCard(const QString &targetCard)
{
    clearError();

    // 验证目标卡号是否有效
    OperationResult result = m_validator.validateTargetAccount(targetCard);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }
    
    return true;
}

/**
 * @brief 获取转账目标卡号的持卡人姓名 (用于显示)
 * @param targetCard 目标卡号
 * @return 目标卡号的持卡人姓名，如果无效返回空字符串
 */
QString AccountViewModel::getTargetCardHolderName(const QString &targetCard)
{
    // 使用repository直接获取账户信息
    std::optional<Account> accountOpt = m_accountModel.getRepository()->findByCardNumber(targetCard);
    if (!accountOpt) {
        return QString();
    }
    
    return accountOpt.value().holderName;
}

/**
 * @brief 处理修改账户 PIN 码操作
 * @param currentPin 当前 PIN 码
 * @param newPin 新 PIN 码
 * @param confirmPin 确认新 PIN 码
 * @return 如果操作成功返回 true，否则返回 false
 */
bool AccountViewModel::changePassword(const QString &currentPin, const QString &newPin, const QString &confirmPin)
{
    clearError();

    // 首先检查登录状态
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        emit transactionCompleted(false, "请先登录"); // 发出失败信号
        return false;
    }

    // 调用 Model 层直接使用changePin方法修改PIN码
    OperationResult result = m_accountModel.changePin(m_cardNumber, currentPin, newPin, confirmPin);
    if (result.success) {
        // 移除PIN码修改交易记录
        // recordTransaction(TransactionType::Other, 0.0, balance(), "修改PIN码成功");

        // 通知 UI 操作完成
        emit transactionCompleted(true, "PIN码修改成功");
        return true;
    } else {
        setErrorMessage(result.errorMessage);
        emit transactionCompleted(false, result.errorMessage); // 发出失败信号
        return false;
    }
}

/**
 * @brief 处理用户登出操作
 */
void AccountViewModel::logout()
{
    if (m_isLoggedIn) {
        // 在清除卡号之前，保存当前卡号和余额用于交易记录
        QString oldCardNumber = m_cardNumber;
        double oldBalance = balance();

        // 移除登出交易记录
        // if (!oldCardNumber.isEmpty()) {
        //     recordTransaction(TransactionType::Other, 0.0, oldBalance, "登出系统");
        // }

        // 重置视图状态
        m_isLoggedIn = false;
        m_isAdmin = false;
        m_cardNumber.clear();
        m_errorMessage.clear();

        // 发出信号通知 UI
        emit isLoggedInChanged();
        emit isAdminChanged();
        emit cardNumberChanged();
        emit errorMessageChanged();
        emit loggedOut(); // 发出登出特有信号

        qDebug() << "成功登出系统";

        // 登出时重置预测余额
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
    }
}

/**
 * @brief 清除当前的错误信息
 */
void AccountViewModel::clearError()
{
    if (!m_errorMessage.isEmpty()) {
        m_errorMessage.clear();
        emit errorMessageChanged(); // 通知 UI 属性改变
    }
}

/**
 * @brief 辅助方法：记录交易
 * @param type 交易类型
 * @param amount 交易金额
 * @param balanceAfter 交易后余额
 * @param description 交易描述
 * @param targetCard 目标卡号 (转账时使用)
 */
void AccountViewModel::recordTransaction(TransactionType type, double amount, double balanceAfter, const QString &description, const QString &targetCard)
{
    // 直接使用 TransactionModel 的方法记录交易
    if (m_transactionModel) {
        m_transactionModel->recordTransaction(
            m_cardNumber, 
            type, 
            amount, 
            balanceAfter, 
            description, 
            targetCard
        );
        
        // 触发信号通知交易视图模型更新
        emit transactionRecorded();
    }
    else {
        qWarning() << "交易模型未设置，无法记录交易";
    }
}

/**
 * @brief 设置当前的错误信息
 * @param message 错误信息字符串
 */
void AccountViewModel::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
    emit errorMessageChanged(); // 通知 UI 属性改变
}

/**
 * @brief 计算预测余额
 * @param daysInFuture 预测未来天数 (默认为 7 天)
 */
void AccountViewModel::calculatePredictedBalance(int daysInFuture)
{
    // 添加详细日志
    qDebug() << "开始计算预测余额, 卡号:" << m_cardNumber
             << "天数:" << daysInFuture
             << "TransactionModel 是否为空:" << (m_transactionModel == nullptr);

    // 初始化预测余额变量
    double newPredictedBalance = 0.0;

    // 调用 Model 层验证和计算预测余额
    OperationResult result = m_accountModel.calculatePredictedBalance(
        m_cardNumber, daysInFuture, newPredictedBalance);

    // 如果计算失败，输出警告并可能重置预测余额
    if (!result.success) {
        qWarning() << "预测余额计算失败:" << result.errorMessage;
        setErrorMessage(result.errorMessage); // 设置错误信息以便 UI 显示
        if (m_predictedBalance != 0.0) {
            m_predictedBalance = 0.0;
            emit predictedBalanceChanged();
        }
        return;
    }

    // 更新预测余额并发送通知
    if (m_predictedBalance != newPredictedBalance) {
        m_predictedBalance = newPredictedBalance;
        emit predictedBalanceChanged();
    }

    qDebug() << "预测余额计算成功, 结果:" << m_predictedBalance;
}

/**
 * @brief 计算多日期预测余额
 * 预测未来多个时间点的余额变化趋势
 * @param days 预测天数列表，如 "7,14,30,90" 字符串形式
 */
void AccountViewModel::calculateMultiDayPredictions(const QString &days)
{
    // 添加详细日志
    qDebug() << "开始计算多日期预测余额, 卡号:" << m_cardNumber
             << "天数列表:" << days;

    // 如果没有登录或者卡号为空，则返回
    if (!m_isLoggedIn || m_cardNumber.isEmpty()) {
        setErrorMessage("请先登录");
        return;
    }

    // 将字符串转换为整数列表
    QVector<int> daysList;
    QStringList daysStrList = days.split(",", Qt::SkipEmptyParts);
    for (const QString &dayStr : daysStrList) {
        bool ok;
        int day = dayStr.trimmed().toInt(&ok);
        if (ok && day > 0) {
            daysList.append(day);
        }
    }

    // 如果没有有效的预测天数，则返回
    if (daysList.isEmpty()) {
        setErrorMessage("请提供有效的预测天数列表");
        return;
    }

    // 存储各时间点的预测余额
    QMap<int, double> predictions;
    
    // 调用 Model 层的多日期预测方法
    OperationResult result = m_accountModel.predictBalanceMultiDays(
        m_cardNumber, daysList, predictions);

    // 如果计算失败，输出警告
    if (!result.success) {
        qWarning() << "多日期预测余额计算失败:" << result.errorMessage;
        setErrorMessage(result.errorMessage);
        return;
    }

    // 将结果转换为QVariantMap方便QML访问
    QVariantMap predictionMap;
    for (auto it = predictions.constBegin(); it != predictions.constEnd(); ++it) {
        predictionMap[QString::number(it.key())] = it.value();
    }

    // 更新多日期预测结果并发送通知
    m_multiDayPredictions = predictionMap;
    emit multiDayPredictionsChanged();

    // 同时更新单日期预测结果（默认使用7天）
    if (daysList.contains(7)) {
        double prediction7days = predictions.value(7, 0.0);
        if (m_predictedBalance != prediction7days) {
            m_predictedBalance = prediction7days;
            emit predictedBalanceChanged();
        }
    }

    qDebug() << "多日期预测余额计算成功, 结果:" << m_multiDayPredictions;
}

// --- 管理员方法实现 (可调用) ---

/**
 * @brief 获取所有账户列表 (管理员权限)
 * @return 包含所有账户数据的 QVariantList
 */
QVariantList AccountViewModel::getAllAccounts()
{
    // 仅允许管理员获取所有账户信息
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return QVariantList();
    }

    // 直接调用 Model 层的方法获取账户列表
    return m_accountModel.getAllAccountsAsVariantList();
}

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
bool AccountViewModel::createAccount(const QString &cardNumber, const QString &pin, const QString &holderName,
                                   double balance, double withdrawLimit, bool isLocked, bool isAdmin)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("创建账户需要管理员权限");
        return false;
    }

    // 验证创建账户参数
    OperationResult result = m_validator.validateCreateAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 执行创建账户操作
    OperationResult createResult = m_accountModel.createAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (createResult.success) {
        // 如果需要锁定账户，额外进行锁定操作
        if (isLocked) {
            m_accountModel.setAccountLockStatus(cardNumber, true);
        }

        emit accountsChanged(); // 通知UI账户列表已更改
        
        emit transactionCompleted(true, QString("成功创建账户 %1").arg(cardNumber));
        return true;
    } else {
        setErrorMessage(createResult.errorMessage);
        return false;
    }
}

/**
 * @brief 更新现有账户信息 (管理员权限)
 * @param cardNumber 卡号
 * @param holderName 持卡人姓名
 * @param balance 余额
 * @param withdrawLimit 取款限额
 * @param isLocked 是否锁定
 * @return 如果成功更新返回 true，否则返回 false
 */
bool AccountViewModel::updateAccount(const QString &cardNumber, const QString &holderName,
                                   double balance, double withdrawLimit, bool isLocked)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("更新账户需要管理员权限");
        return false;
    }

    // 再次验证管理员权限
    OperationResult adminResult = m_validator.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 执行更新账户操作
    OperationResult updateResult = m_accountModel.updateAccount(cardNumber, holderName, balance, withdrawLimit, isLocked);
    if (updateResult.success) {
        // 通知UI账户列表已更改
        emit accountsChanged();
        
        // 如果是更新当前登录账户，还需更新UI显示
        if (cardNumber == m_cardNumber) {
            emit holderNameChanged();
            emit balanceChanged();
            emit withdrawLimitChanged();
        }
        
        emit transactionCompleted(true, QString("成功更新账户 %1").arg(cardNumber));
        return true;
    } else {
        setErrorMessage(updateResult.errorMessage);
        return false;
    }
}

/**
 * @brief 删除账户 (管理员权限)
 * @param cardNumber 要删除的账户卡号
 * @return 如果成功删除返回 true，否则返回 false
 */
bool AccountViewModel::deleteAccount(const QString &cardNumber)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("删除账户需要管理员权限");
        return false;
    }

    // 再次验证管理员权限
    OperationResult adminResult = m_validator.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 不允许删除当前登录账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能删除当前登录的账户");
        return false;
    }

    // 执行删除账户操作
    OperationResult deleteResult = m_accountModel.deleteAccount(cardNumber);
    if (deleteResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        emit transactionCompleted(true, QString("成功删除账户 %1").arg(cardNumber));
        return true;
    } else {
        setErrorMessage(deleteResult.errorMessage);
        return false;
    }
}

/**
 * @brief 重置账户 PIN 码 (管理员权限)
 * @param cardNumber 卡号
 * @param newPin 新 PIN 码
 * @return 如果成功重置返回 true，否则返回 false
 */
bool AccountViewModel::resetAccountPin(const QString &cardNumber, const QString &newPin)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("重置PIN码需要管理员权限");
        return false;
    }

    // 再次验证管理员权限
    OperationResult adminResult = m_validator.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 执行重置PIN码操作
    OperationResult resetResult = m_accountModel.resetPin(cardNumber, newPin);
    if (resetResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        emit transactionCompleted(true, QString("成功重置账户 %1 的PIN码").arg(cardNumber));
        return true;
    } else {
        setErrorMessage(resetResult.errorMessage);
        return false;
    }
}

/**
 * @brief 设置账户锁定状态 (管理员权限)
 * @param cardNumber 卡号
 * @param locked 是否锁定
 * @return 如果成功设置返回 true，否则返回 false
 */
bool AccountViewModel::setAccountLockStatus(const QString &cardNumber, bool locked)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("设置账户锁定状态需要管理员权限");
        return false;
    }

    // 再次验证管理员权限
    OperationResult adminResult = m_validator.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 执行设置账户锁定状态操作
    OperationResult lockResult = m_accountModel.setAccountLockStatus(cardNumber, locked);
    if (lockResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        
        QString statusMsg = locked ? "已锁定" : "已解锁";
        emit transactionCompleted(true, QString("账户 %1 %2").arg(cardNumber).arg(statusMsg));
        return true;
    } else {
        setErrorMessage(lockResult.errorMessage);
        return false;
    }
}

/**
 * @brief 设置账户取款限额 (管理员权限)
 * @param cardNumber 卡号
 * @param limit 新的取款限额
 * @return 如果成功设置返回 true，否则返回 false
 */
bool AccountViewModel::setWithdrawLimit(const QString &cardNumber, double limit)
{
    clearError();

    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("设置取款限额需要管理员权限");
        return false;
    }

    // 再次验证管理员权限
    OperationResult adminResult = m_validator.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 执行设置取款限额操作
    OperationResult limitResult = m_accountModel.setWithdrawLimit(cardNumber, limit);
    if (limitResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        
        // 如果是更新当前登录账户，还需更新UI显示
        if (cardNumber == m_cardNumber) {
            emit withdrawLimitChanged();
        }
        
        emit transactionCompleted(true, QString("成功将账户 %1 的取款限额设置为 %2").arg(cardNumber).arg(limit));
        return true;
    } else {
        setErrorMessage(limitResult.errorMessage);
        return false;
    }
}