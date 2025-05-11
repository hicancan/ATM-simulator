// AppController.h
/**
 * @file AppController.h
 * @brief 应用程序控制器头文件
 *
 * 定义了 AppController 类，作为整个应用程序的中心控制器。
 * 负责创建和管理 Model 和 ViewModel 实例，连接信号槽，以及处理页面切换逻辑。
 * 将 ViewModel 暴露给 QML。
 */
#pragma once

#include <QObject>
#include <QQmlEngine> // 包含 QQmlEngine 头文件
#include <QString>
// 包含各个 ViewModel 和 Model 的头文件
#include "viewmodels/AccountViewModel.h"
#include "viewmodels/TransactionViewModel.h"
#include "viewmodels/PrinterViewModel.h"
#include "models/TransactionModel.h" // AppController 需要创建 TransactionModel
#include "models/JsonPersistenceManager.h" // 添加JsonPersistenceManager头文件

// 将类型声明为Qt元对象系统的已知类型，以便QML可以使用这些类型
Q_DECLARE_METATYPE(AccountViewModel*)
Q_DECLARE_METATYPE(TransactionViewModel*)
Q_DECLARE_METATYPE(PrinterViewModel*)

/**
 * @brief 应用程序控制器类
 *
 * 整个应用程序的核心控制器，协调 Model、ViewModel 和 UI 之间的交互。
 * 创建并持有 ViewModel 实例，通过 ContextProperty 暴露给 QML。
 */
class AppController : public QObject
{
    Q_OBJECT

    // Q_PROPERTY 宏将 ViewModel 实例暴露给 QML 作为属性 (常量属性，只读)
    Q_PROPERTY(AccountViewModel* accountViewModel READ accountViewModel CONSTANT)
    Q_PROPERTY(TransactionViewModel* transactionViewModel READ transactionViewModel CONSTANT)
    Q_PROPERTY(PrinterViewModel* printerViewModel READ printerViewModel CONSTANT)
    // Q_PROPERTY 暴露当前页面名称，用于控制 UI 页面切换
    Q_PROPERTY(QString currentPage READ currentPage NOTIFY currentPageChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit AppController(QObject *parent = nullptr);
    /**
     * @brief 析构函数
     */
    ~AppController();

    /**
     * @brief 初始化控制器
     *
     * 在应用程序启动时调用，用于注册 QML 类型和设置初始状态。
     *
     * @param engine QQmlEngine 指针
     */
    void initialize(QQmlEngine* engine);

    // --- 属性获取方法 ---
    /**
     * @brief 获取 AccountViewModel 实例指针
     * @return AccountViewModel 指针
     */
    AccountViewModel* accountViewModel() const;
    /**
     * @brief 获取 TransactionViewModel 实例指针
     * @return TransactionViewModel 指针
     */
    TransactionViewModel* transactionViewModel() const;
    /**
     * @brief 获取 PrinterViewModel 实例指针
     * @return PrinterViewModel 指针
     */
    PrinterViewModel* printerViewModel() const;
    /**
     * @brief 获取当前页面名称
     * @return 当前页面的名称字符串
     */
    QString currentPage() const;

    // --- 可调用方法 (供 QML 调用) ---
    /**
     * @brief 切换到指定的页面
     * @param pageName 要切换到的页面名称
     */
    Q_INVOKABLE void switchToPage(const QString &pageName);
    /**
     * @brief 处理用户登出操作
     */
    Q_INVOKABLE void logout();

signals:
    /**
     * @brief 当前页面名称改变时发出的信号
     */
    void currentPageChanged();

private:
    //!< 当前页面名称
    QString m_currentPage = "LoginPage";
    //!< JsonPersistenceManager 实例指针 (由 AppController 持有并注入到存储库)
    JsonPersistenceManager* m_persistenceManager;
    //!< AccountViewModel 实例指针
    AccountViewModel* m_accountViewModel;
    //!< TransactionViewModel 实例指针
    TransactionViewModel* m_transactionViewModel;
    //!< PrinterViewModel 实例指针
    PrinterViewModel* m_printerViewModel;
    //!< TransactionModel 实例指针 (由 AppController 持有并注入到 ViewModel)
    TransactionModel* m_transactionModel;
};