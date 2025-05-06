#include "AppController.h"
#include "viewmodels/AccountViewModel.h"
#include "viewmodels/TransactionViewModel.h"
#include "viewmodels/PrinterViewModel.h"
#include "models/TransactionModel.h"
#include <QDebug>
#include <QQmlComponent>

AppController::AppController(QObject *parent)
    : QObject(parent)
    , m_accountViewModel(new AccountViewModel(this))
    , m_transactionViewModel(new TransactionViewModel(this))
    , m_printerViewModel(new PrinterViewModel(this))
{
    // Connect signals
    connect(m_accountViewModel, &AccountViewModel::loggedOut, 
            this, [this]() { switchToPage("LoginPage"); });
    
    // 添加连接，当交易记录被添加时刷新交易视图模型
    connect(m_accountViewModel, &AccountViewModel::transactionRecorded,
            m_transactionViewModel, &TransactionViewModel::refreshTransactions);
    
    // Create shared transaction model and connect to both view models
    TransactionModel* transactionModel = new TransactionModel(this);
    m_accountViewModel->setTransactionModel(transactionModel);
    m_transactionViewModel->setTransactionModel(transactionModel);
}

AppController::~AppController()
{
    // QObject parent-child relationship handles cleanup
}

void AppController::initialize(QQmlEngine* engine)
{
    // Register types with QML
    qmlRegisterType<AccountViewModel>("ATMSimulator", 1, 0, "AccountViewModel");
    qmlRegisterType<TransactionViewModel>("ATMSimulator", 1, 0, "TransactionViewModel");
    qmlRegisterType<PrinterViewModel>("ATMSimulator", 1, 0, "PrinterViewModel");
    
    // 确保组件目录被正确加载
    qDebug() << "组件路径: " << engine->importPathList();
    
    // 预加载重要组件，确保不会出现找不到组件的问题
    QQmlComponent component(engine, QUrl("qrc:/qml/components/ReceiptPrinter.qml"));
    if (component.isError()) {
        qDebug() << "组件加载错误:" << component.errorString();
    }
}

AccountViewModel* AppController::accountViewModel() const
{
    return m_accountViewModel;
}

TransactionViewModel* AppController::transactionViewModel() const
{
    return m_transactionViewModel;
}

PrinterViewModel* AppController::printerViewModel() const
{
    return m_printerViewModel;
}

QString AppController::currentPage() const
{
    return m_currentPage;
}

void AppController::switchToPage(const QString &pageName)
{
    if (m_currentPage != pageName) {
        // 如果要切换到交易历史页面，确保交易视图模型有正确的卡号
        if (pageName == "TransactionHistoryPage" && m_accountViewModel->isLoggedIn()) {
            QString cardNumber = m_accountViewModel->cardNumber();
            // 先设置为空再重新设置，强制刷新
            m_transactionViewModel->setCardNumber("");
            m_transactionViewModel->setCardNumber(cardNumber);
            m_transactionViewModel->refreshTransactions();
            qDebug() << "切换到交易历史页面，已刷新交易记录，卡号:" << cardNumber;
        }
        
        m_currentPage = pageName;
        emit currentPageChanged();
    }
}

void AppController::logout()
{
    m_accountViewModel->logout();
    switchToPage("LoginPage");
} 