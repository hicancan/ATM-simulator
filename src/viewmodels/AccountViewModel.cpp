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
    , m_isLoggedIn(false)
    , m_predictedBalance(0.0)
    , m_isAdmin(false)
    , m_cardNumber("")
    , m_errorMessage("")
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

    // 基本输入验证，只检查输入值不为空
    if (m_cardNumber.isEmpty()) {
        setErrorMessage("请输入卡号");
        return false;
    }

    if (pin.isEmpty()) {
        setErrorMessage("请输入PIN码");
        return false;
    }

    // 直接调用 Model 层执行登录
    LoginResult loginResult = m_accountModel.performLogin(m_cardNumber, pin);
    if (loginResult.success) {
        m_isLoggedIn = true;
        m_isAdmin = loginResult.isAdmin;
        
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

    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 直接调用Model层的方法，由Model层统一处理所有验证
    OperationResult withdrawResult = m_accountModel.withdrawAmount(m_cardNumber, amount);
    if (withdrawResult.success) {
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        return handleOperationResult(withdrawResult, QString("成功取款 %1 元").arg(amount));
    } else {
        return handleOperationResult(withdrawResult, "");
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

    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 直接调用Model层的方法，由Model层统一处理所有验证
    OperationResult depositResult = m_accountModel.depositAmount(m_cardNumber, amount);
    if (depositResult.success) {
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        return handleOperationResult(depositResult, QString("成功存款 %1 元").arg(amount));
    } else {
        return handleOperationResult(depositResult, "");
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

    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 直接调用Model层的方法，由Model层统一处理所有验证
    OperationResult transferResult = m_accountModel.transferAmount(m_cardNumber, targetCard, amount);
    if (transferResult.success) {
        // 通知余额变化
        emit balanceChanged();
        
        // 操作完成通知
        return handleOperationResult(transferResult, QString("成功转账 %1 元到账户 %2").arg(amount).arg(targetCard));
    } else {
        return handleOperationResult(transferResult, "");
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

    // 基本输入验证
    if (targetCard.isEmpty()) {
        setErrorMessage("请输入目标卡号");
        return false;
    }

    // 使用Model层验证目标卡号
    OperationResult result = m_accountModel.validateTargetAccount(targetCard);
    return handleOperationResult(result, "");
}

/**
 * @brief 获取转账目标卡号的持卡人姓名 (用于显示)
 * @param targetCard 目标卡号
 * @return 目标卡号的持卡人姓名，如果无效返回空字符串
 */
QString AccountViewModel::getTargetCardHolderName(const QString &targetCard)
{
    // 使用AccountModel提供的方法，不直接访问repository
    return m_accountModel.getTargetCardHolderName(targetCard);
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

    // 基本输入验证
    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    if (currentPin.isEmpty()) {
        setErrorMessage("请输入当前PIN码");
        return false;
    }

    if (newPin.isEmpty()) {
        setErrorMessage("请输入新PIN码");
        return false;
    }

    if (confirmPin.isEmpty()) {
        setErrorMessage("请确认新PIN码");
        return false;
    }

    if (newPin != confirmPin) {
        setErrorMessage("两次输入的新PIN码不匹配");
        return false;
    }

    // 执行修改PIN码操作，所有业务验证由Model层处理
    OperationResult changeResult = m_accountModel.changePin(m_cardNumber, currentPin, newPin, confirmPin);
    return handleOperationResult(changeResult, "PIN码修改成功");
}

/**
 * @brief 处理用户登出操作
 */
void AccountViewModel::logout()
{
    if (m_isLoggedIn) {
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
 * @brief 设置当前的错误信息
 * @param message 错误信息字符串
 */
void AccountViewModel::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
    emit errorMessageChanged(); // 通知 UI 属性改变
}

/**
 * @brief 检查管理员权限
 * @param errorMsg 如果权限检查失败时显示的错误消息
 * @return 如果当前用户有管理员权限返回true，否则返回false
 */
bool AccountViewModel::checkAdminPermission(const QString& errorMsg)
{
    // 验证管理员权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage(errorMsg);
        return false;
    }
    
    // 使用Model层检查当前卡号是否具有管理员权限
    OperationResult adminOperationResult = m_accountModel.checkAdminPermission(m_cardNumber);
    if (!adminOperationResult.success) {
        setErrorMessage(adminOperationResult.errorMessage);
        return false;
    }
    
    return true;
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
    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("没有权限执行此操作")) {
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("创建账户需要管理员权限")) {
        return false;
    }

    // 基本输入验证
    if (cardNumber.isEmpty() || pin.isEmpty() || holderName.isEmpty()) {
        setErrorMessage("卡号、PIN码和持卡人姓名不能为空");
        return false;
    }

    if (balance < 0 || withdrawLimit <= 0) {
        setErrorMessage("余额不能为负，取款限额必须为正数");
        return false;
    }

    // 执行创建账户操作，业务验证由Model层处理
    OperationResult createResult = m_accountModel.createAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (createResult.success) {
        // 如果需要锁定账户，额外进行锁定操作
        if (isLocked) {
            m_accountModel.setAccountLockStatus(cardNumber, true);
        }

        emit accountsChanged(); // 通知UI账户列表已更改
        
        return handleOperationResult(createResult, QString("成功创建账户 %1").arg(cardNumber));
    } else {
        return handleOperationResult(createResult, "");
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("更新账户需要管理员权限")) {
        return false;
    }

    // 基本输入验证
    if (cardNumber.isEmpty() || holderName.isEmpty()) {
        setErrorMessage("卡号和持卡人姓名不能为空");
        return false;
    }

    if (balance < 0 || withdrawLimit <= 0) {
        setErrorMessage("余额不能为负，取款限额必须为正数");
        return false;
    }

    // 执行更新账户操作，业务验证由Model层处理
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
        
        return handleOperationResult(updateResult, QString("成功更新账户 %1").arg(cardNumber));
    } else {
        return handleOperationResult(updateResult, "");
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("删除账户需要管理员权限")) {
        return false;
    }

    // 基本输入验证 - 不能删除当前登录账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能删除当前登录的账户");
        return false;
    }

    // 执行删除账户操作，业务验证由Model层处理
    OperationResult deleteResult = m_accountModel.deleteAccount(cardNumber);
    if (deleteResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        return handleOperationResult(deleteResult, QString("成功删除账户 %1").arg(cardNumber));
    } else {
        return handleOperationResult(deleteResult, "");
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("重置PIN码需要管理员权限")) {
        return false;
    }

    // 基本输入验证
    if (cardNumber.isEmpty() || newPin.isEmpty()) {
        setErrorMessage("卡号和新PIN码不能为空");
        return false;
    }

    // 执行重置PIN码操作，业务验证由Model层处理
    OperationResult resetResult = m_accountModel.resetPin(cardNumber, newPin);
    if (resetResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        return handleOperationResult(resetResult, QString("成功重置账户 %1 的PIN码").arg(cardNumber));
    } else {
        return handleOperationResult(resetResult, "");
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("设置账户锁定状态需要管理员权限")) {
        return false;
    }

    // 基本输入验证
    if (cardNumber.isEmpty()) {
        setErrorMessage("卡号不能为空");
        return false;
    }

    // 执行设置账户锁定状态操作，业务验证由Model层处理
    OperationResult lockResult = m_accountModel.setAccountLockStatus(cardNumber, locked);
    if (lockResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        
        QString statusMsg = locked ? "已锁定" : "已解锁";
        return handleOperationResult(lockResult, QString("账户 %1 %2").arg(cardNumber).arg(statusMsg));
    } else {
        return handleOperationResult(lockResult, "");
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

    // 使用通用的管理员权限检查方法
    if (!checkAdminPermission("设置取款限额需要管理员权限")) {
        return false;
    }

    // 基本输入验证
    if (cardNumber.isEmpty()) {
        setErrorMessage("卡号不能为空");
        return false;
    }

    if (limit <= 0) {
        setErrorMessage("取款限额必须为正数");
        return false;
    }

    // 执行设置取款限额操作，业务验证由Model层处理
    OperationResult limitResult = m_accountModel.setWithdrawLimit(cardNumber, limit);
    if (limitResult.success) {
        emit accountsChanged(); // 通知UI账户列表已更改
        
        // 如果是更新当前登录账户，还需更新UI显示
        if (cardNumber == m_cardNumber) {
            emit withdrawLimitChanged();
        }
        
        return handleOperationResult(limitResult, QString("成功将账户 %1 的取款限额设置为 %2").arg(cardNumber).arg(limit));
    } else {
        return handleOperationResult(limitResult, "");
    }
}

/**
 * @brief 处理操作结果，设置错误信息并发送完成信号
 * @param result 操作结果
 * @param successMessage 操作成功时的消息
 * @return 如果操作成功返回true，否则返回false
 */
bool AccountViewModel::handleOperationResult(const OperationResult& result, const QString& successMessage)
{
    if (result.success) {
        // 清除错误信息
        clearError();
        // 发送操作完成信号
        emit transactionCompleted(true, successMessage);
        return true;
    } else {
        // 设置错误信息
        setErrorMessage(result.errorMessage);
        // 发送操作完成信号
        emit transactionCompleted(false, result.errorMessage);
        return false;
    }
}