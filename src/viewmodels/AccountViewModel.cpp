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
    OperationResult result = m_accountModel.validateCredentials(m_cardNumber, pin);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 登录成功，执行登录过程获取账户详情
    LoginResult loginResult = m_accountModel.performLogin(m_cardNumber, pin);
    if (loginResult.success) {
        m_isLoggedIn = true;
        m_isAdmin = loginResult.isAdmin;
        
        // 记录登录交易
        recordTransaction(TransactionType::Other, 0.0, loginResult.balance, "登录系统");

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
        emit transactionCompleted(false, "请先登录"); // 发出失败信号
        return false;
    }

    // 调用 Model 层验证取款操作
    OperationResult result = m_accountModel.validateWithdrawal(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        emit transactionCompleted(false, result.errorMessage); // 发出失败信号
        return false;
    }

    // 调用 Model 层执行取款操作
    result = m_accountModel.withdrawAmount(m_cardNumber, amount);
    if (result.success) {
        // 更新余额并通知 UI
        emit balanceChanged();

        // 记录交易
        recordTransaction(TransactionType::Withdrawal, amount, balance(), "取款成功");

        // 通知 UI 操作完成
        emit transactionCompleted(true, QString("成功取款：￥%1").arg(amount));
        return true;
    } else {
        // 处理取款失败
        setErrorMessage(result.errorMessage.isEmpty() ? "取款失败，请重试" : result.errorMessage);
        emit transactionCompleted(false, m_errorMessage); // 发出失败信号
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
        emit transactionCompleted(false, "请先登录"); // 发出失败信号
        return false;
    }

    // 调用 Model 层验证存款操作
    OperationResult result = m_accountModel.validateDeposit(m_cardNumber, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        emit transactionCompleted(false, result.errorMessage); // 发出失败信号
        return false;
    }

    // 调用 Model 层执行存款操作
    result = m_accountModel.depositAmount(m_cardNumber, amount);
    if (result.success) {
        // 更新余额并通知 UI
        emit balanceChanged();

        // 记录交易
        recordTransaction(TransactionType::Deposit, amount, balance(), "存款成功");

        // 通知 UI 操作完成
        emit transactionCompleted(true, QString("成功存款：￥%1").arg(amount));
        return true;
    } else {
        // 处理存款失败
        setErrorMessage(result.errorMessage.isEmpty() ? "存款失败，请重试" : result.errorMessage);
        emit transactionCompleted(false, m_errorMessage); // 发出失败信号
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
        emit transactionCompleted(false, "请先登录"); // 发出失败信号
        return false;
    }

    // 调用 Model 层验证转账操作
    OperationResult result = m_accountModel.validateTransfer(m_cardNumber, targetCard, amount);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        emit transactionCompleted(false, result.errorMessage); // 发出失败信号
        return false;
    }

    // 调用 Model 层执行转账操作
    result = m_accountModel.transferAmount(m_cardNumber, targetCard, amount);
    if (result.success) {
        // 更新源账户余额并通知 UI
        emit balanceChanged();

        // 获取接收方姓名用于交易描述
        QString receiverName = m_accountModel.getHolderName(targetCard);
        QString description = QString("转账至%1（%2）").arg(receiverName).arg(targetCard.right(4));

        // 记录转账方的交易
        recordTransaction(TransactionType::Transfer, amount, balance(), description, targetCard);

        // 记录接收方的交易（由 AccountModel 调用 TransactionModel 完成）
        try {
            // 获取接收方当前余额
            double receiverBalance = m_accountModel.getBalance(targetCard);

            // 只有当获取目标账户余额成功时才记录交易
            if (receiverBalance >= 0) { // 假设 getBalance 返回 >= 0 表示成功
                // 构建收款方交易描述
                QString receiveDescription = QString("收到来自%1（%2）的转账").arg(holderName()).arg(m_cardNumber.right(4));

                // 通过 AccountModel 记录收款方交易
                m_accountModel.recordTransaction(
                    targetCard,
                    TransactionType::Deposit, // 收到的转账对收款方是存款
                    amount,
                    receiverBalance,
                    receiveDescription,
                    m_cardNumber // 记录发送方卡号
                );
            } else {
                qWarning() << "无法记录收款方交易，目标账户余额无效:" << targetCard;
            }
        } catch (const std::exception& e) {
            qWarning() << "记录收款方交易时发生异常:" << e.what();
        } catch (...) {
            qWarning() << "记录收款方交易时发生未知异常";
        }


        // 通知 UI 操作完成
        emit transactionCompleted(true, QString("成功转账：￥%1 至 %2").arg(amount).arg(receiverName));
        return true;
    } else {
        // 处理转账失败
        setErrorMessage(result.errorMessage.isEmpty() ? "转账失败，请重试" : result.errorMessage);
        emit transactionCompleted(false, m_errorMessage); // 发出失败信号
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

    if (!m_isLoggedIn) {
        setErrorMessage("请先登录");
        return false;
    }

    // 调用 Model 层专门验证目标账户的方法
    OperationResult result = m_accountModel.validateTargetAccount(targetCard);
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
    // 调用 Model 层获取目标账户信息
    QString holderName;
    bool isLocked;
    
    if (m_accountModel.getTargetAccountInfo(targetCard, holderName, isLocked)) {
        return holderName;
    }
    
    return QString();
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
        // 记录交易
        recordTransaction(TransactionType::Other, 0.0, balance(), "修改PIN码成功");

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

        // 记录登出交易 (仅当卡号有效时)
        if (!oldCardNumber.isEmpty()) {
            recordTransaction(TransactionType::Other, 0.0, oldBalance, "登出系统");
        }

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
 * @brief 辅助方法：使用 AccountModel 记录交易
 * @param type 交易类型
 * @param amount 交易金额
 * @param balanceAfter 交易后余额
 * @param description 交易描述
 * @param targetCard 目标卡号 (转账时使用)
 */
void AccountViewModel::recordTransaction(TransactionType type, double amount, double balanceAfter, const QString &description, const QString &targetCard)
{
    // 使用 AccountModel 的方法记录交易
    // AccountModel 会在内部使用 TransactionModel
    m_accountModel.recordTransaction(m_cardNumber, type, amount, balanceAfter, description, targetCard);

    // 触发信号通知交易视图模型更新
    emit transactionRecorded();
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
 * @brief 获取预测余额属性
 * @return 预测余额
 */
double AccountViewModel::predictedBalance() const
{
    return m_predictedBalance;
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

    // 检查权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }

    // 调用 Model 层验证创建账户参数
    OperationResult result = m_accountModel.validateCreateAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (!result.success) {
        setErrorMessage(result.errorMessage);
        return false;
    }

    // 创建账户对象
    Account account;
    account.cardNumber = cardNumber;
    account.pin = pin;
    account.holderName = holderName;
    account.balance = balance;
    account.withdrawLimit = withdrawLimit;
    account.isLocked = isLocked;
    account.isAdmin = isAdmin;

    // 调用 Model 层创建账户
    result = m_accountModel.createAccount(cardNumber, pin, holderName, balance, withdrawLimit, isAdmin);
    if (result.success) {
        emit accountsChanged(); // 通知 UI 账户列表可能已更改
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "创建账户失败，可能卡号已存在" : result.errorMessage);
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

    // 检查权限
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }

    // 调用 Model 层方法直接更新账户
    OperationResult result = m_accountModel.updateAccount(cardNumber, holderName, balance, withdrawLimit, isLocked);
    if (result.success) {
        emit accountsChanged(); // 通知 UI 账户列表可能已更改

        // 如果更新的是当前登录用户，刷新 UI 数据
        if (cardNumber == m_cardNumber) {
            emit holderNameChanged();
            emit balanceChanged();
            emit withdrawLimitChanged();
            // 注意：当前登录管理员的 isAdmin 和 isLocked 状态变化未在此处明确处理，
            // 如果 UI 需要，可能需要额外的逻辑。
        }

        return true;
    } else {
        setErrorMessage(result.errorMessage);
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

    // 调用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 不能删除当前登录的账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能删除当前登录账户");
        return false;
    }

    // 调用 Model 层删除账户
    OperationResult result = m_accountModel.deleteAccount(cardNumber);
    if (result.success) {
        emit accountsChanged(); // 通知 UI 账户列表可能已更改
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "删除账户失败，可能卡号不存在" : result.errorMessage);
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

    // 仅允许管理员重置 PIN 码
    if (!m_isLoggedIn || !m_isAdmin) {
        setErrorMessage("没有权限执行此操作");
        return false;
    }

    // 调用 Model 层的 resetPin 方法
    OperationResult result = m_accountModel.resetPin(cardNumber, newPin);
    if (result.success) {
        emit accountsChanged(); // PIN 码修改不直接影响列表模型数据，但最好发送信号
        return true;
    } else {
        setErrorMessage(result.errorMessage);
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

    // 调用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 不能锁定当前登录的账户
    if (cardNumber == m_cardNumber) {
        setErrorMessage("不能锁定当前登录账户");
        return false;
    }

    // 验证输入 (检查账户是否存在)
    if (!m_accountModel.accountExists(cardNumber)) {
        setErrorMessage("卡号不存在");
        return false;
    }

    // 调用 Model 层设置锁定状态
    OperationResult result = m_accountModel.setAccountLockStatus(cardNumber, locked);
    if (result.success) {
        emit accountsChanged(); // 通知 UI 账户列表可能已更改
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? (locked ? "锁定账户失败" : "解锁账户失败") : result.errorMessage);
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

    // 调用 Model 层验证管理员权限
    OperationResult adminResult = m_accountModel.validateAdminOperation(m_cardNumber);
    if (!adminResult.success) {
        setErrorMessage(adminResult.errorMessage);
        return false;
    }

    // 验证限额是否合理
    if (limit < 0) {
        setErrorMessage("取款限额不能为负数");
        return false;
    }

    // 调用 Model 层设置取款限额
    OperationResult result = m_accountModel.setWithdrawLimit(cardNumber, limit);
    if (result.success) {
        emit accountsChanged(); // 通知 UI 账户列表可能已更改

        // 如果更新的是当前登录用户，刷新 UI 数据
        if (cardNumber == m_cardNumber) {
            emit withdrawLimitChanged();
        }
        return true;
    } else {
        setErrorMessage(result.errorMessage.isEmpty() ? "设置取款限额失败" : result.errorMessage);
        return false;
    }
}