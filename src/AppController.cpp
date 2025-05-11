// AppController.cpp
/**
 * @file AppController.cpp
 * @brief 应用程序控制器实现文件
 *
 * 实现了 AppController 类中定义的初始化、属性获取和可调用方法。
 * 负责创建和连接 Model/ViewModel 对象，并在 QML 中暴露它们。
 */
#include "AppController.h"
#include "viewmodels/AccountViewModel.h"
#include "viewmodels/TransactionViewModel.h"
#include "viewmodels/PrinterViewModel.h"
#include "models/JsonAccountRepository.h"
#include <QDebug>
#include <QQmlComponent> // 包含 QQmlComponent 头文件

/**
 * @brief 构造函数
 * @param parent 父对象
 */
AppController::AppController(QObject *parent)
    : QObject(parent)
{
    // 首先创建持久化管理器，它将被其他组件使用
    m_persistenceManager = new JsonPersistenceManager(this);
    
    // 创建账户存储库，使用持久化管理器
    JsonAccountRepository* accountRepository = new JsonAccountRepository(m_persistenceManager, "accounts.json");
    
    // 创建交易模型，使用持久化管理器
    m_transactionModel = new TransactionModel(m_persistenceManager, "transactions.json", this);
    
    // 创建 ViewModel 实例，并设置 AppController 为它们的父对象
    m_accountViewModel = new AccountViewModel(this);
    m_transactionViewModel = new TransactionViewModel(this);
    m_printerViewModel = new PrinterViewModel(this);

    // 连接信号槽
    // 当 AccountViewModel 发出 loggedOut 信号时，切换页面到 LoginLoginPage
    connect(m_accountViewModel, &AccountViewModel::loggedOut,
            this, [this]() { switchToPage("LoginPage"); });

    // 连接 AccountViewModel 的 transactionCompleted 信号到 TransactionViewModel 的 refreshTransactions 槽
    // 确保当有新交易记录时，交易历史界面能及时刷新
    connect(m_accountViewModel, &AccountViewModel::transactionCompleted,
            m_transactionViewModel, &TransactionViewModel::refreshTransactions);

    // 将 TransactionModel 实例注入到 AccountModel 中
    // AccountModel 需要 TransactionModel 来记录交易和预测余额
    m_accountViewModel->setTransactionModel(m_transactionModel);

    // 将同一个 TransactionModel 实例也设置到 TransactionViewModel
    // TransactionViewModel 需要 TransactionModel 来获取交易记录并格式化
    m_transactionViewModel->setTransactionModel(m_transactionModel);
}

/**
 * @brief 析构函数
 *
 * QObject 的父子关系会自动处理子对象的内存释放，因此在此无需手动删除 ViewModel 和 Model 指针。
 */
AppController::~AppController()
{
    // JsonPersistenceManager现在是QObject的子对象，会自动被父对象删除
    // QObject 父子关系会自动处理内存清理
}

/**
 * @brief 初始化控制器
 *
 * 在应用程序启动时调用，用于注册 QML 类型和设置初始状态。
 *
 * @param engine QQmlEngine 指针
 */
void AppController::initialize(QQmlEngine* engine)
{
    // 将 ViewModel 类型注册到 QML，使其可以在 QML 中创建和使用
    qmlRegisterType<AccountViewModel>("ATMSimulator", 1, 0, "AccountViewModel");
    qmlRegisterType<TransactionViewModel>("ATMSimulator", 1, 0, "TransactionViewModel");
    qmlRegisterType<PrinterViewModel>("ATMSimulator", 1, 0, "PrinterViewModel");

    // 确保组件目录被正确加载（尽管 main.cpp 中已设置，这里再确认一下）
    qDebug() << "组件路径: " << engine->importPathList();

    // 预加载重要组件，确保不会出现找不到组件的问题（可选，取决于具体 QML 结构）
    QQmlComponent component(engine, QUrl("qrc:/qml/components/ReceiptPrinter.qml"));
    if (component.isError()) {
        qDebug() << "组件加载错误:" << component.errorString();
    }
}

/**
 * @brief 获取 AccountViewModel 实例指针
 * @return AccountViewModel 指针
 */
AccountViewModel* AppController::accountViewModel() const
{
    return m_accountViewModel;
}

/**
 * @brief 获取 TransactionViewModel 实例指针
 * @return TransactionViewModel 指针
 */
TransactionViewModel* AppController::transactionViewModel() const
{
    return m_transactionViewModel;
}

/**
 * @brief 获取 PrinterViewModel 实例指针
 * @return PrinterViewModel 指针
 */
PrinterViewModel* AppController::printerViewModel() const
{
    return m_printerViewModel;
}

/**
 * @brief 获取当前页面名称
 * @return 当前页面的名称字符串
 */
QString AppController::currentPage() const
{
    return m_currentPage;
}

/**
 * @brief 切换到指定的页面
 *
 * 更新当前页面状态，并发出信号通知 UI 进行页面切换。
 * 对特定页面（如交易历史页面）进行额外处理。
 *
 * @param pageName 要切换到的页面名称
 */
void AppController::switchToPage(const QString &pageName)
{
    // 只有当页面名称不同时才进行切换
    if (m_currentPage != pageName) {
        // 如果要切换到交易历史页面，确保 TransactionViewModel 使用正确的卡号
        if (pageName == "TransactionHistoryPage" && m_accountViewModel->isLoggedIn()) {
            QString cardNumber = m_accountViewModel->cardNumber();
            // 更新 TransactionViewModel 的卡号，这会触发其内部刷新
            // 这里先设置为空再重新设置，以确保即使卡号相同也能强制刷新列表
            m_transactionViewModel->setCardNumber("");
            m_transactionViewModel->setCardNumber(cardNumber);
             qDebug() << "切换到交易历史页面，已设置卡号:" << cardNumber;
        }

        m_currentPage = pageName; // 更新当前页面状态
        emit currentPageChanged(); // 发出信号通知 UI
        qDebug() << "切换到页面:" << pageName;
    }
}

/**
 * @brief 处理用户登出操作
 *
 * 调用 AccountViewModel 的登出方法，然后切换回登录页面。
 */
Q_INVOKABLE void AppController::logout()
{
    m_accountViewModel->logout(); // 调用 ViewModel 的登出逻辑
    switchToPage("LoginPage"); // 切换回登录页面
}