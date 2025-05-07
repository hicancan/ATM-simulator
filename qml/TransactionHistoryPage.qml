import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    id: page
    
    background: Rectangle {
        color: "#1e2029"
    }
    
    // Header
    HeaderBar {
        id: header
        title: "交易历史"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: controller.switchToPage("BalancePage")
    }
    
    // 添加一个可视化的刷新按钮
    Button {
        id: refreshButton
        anchors.top: header.bottom
        anchors.right: parent.right
        anchors.margins: 10
        text: "刷新"
        highlighted: true
        
        onClicked: {
            refreshTransactionHistory()
        }
    }
    
    // 用于显示调试消息
    Label {
        id: debugLabel
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: refreshButton.left
        anchors.margins: 10
        font.pixelSize: 12
        color: "white"
        text: "调试: 准备加载交易记录..."
        visible: true // 设为true以显示调试信息
    }
    
    // 初始化时刷新交易记录
    Component.onCompleted: {
        console.log("TransactionHistoryPage已初始化，即将刷新交易历史")
        Qt.callLater(refreshTransactionHistory)
    }
    
    // 添加函数来刷新交易记录
    function refreshTransactionHistory() {
        console.log("正在刷新交易历史...")
        debugLabel.text = "调试: 正在刷新交易记录..."
        
        // 确保设置了正确的卡号
        if (controller && controller.accountViewModel && controller.accountViewModel.isLoggedIn && controller.accountViewModel.cardNumber) {
            var cardNum = controller.accountViewModel.cardNumber
            debugLabel.text = "调试: 正在获取卡号" + cardNum + "的交易记录..."
            
            // 使用新的updateCardNumber方法替代setCardNumber方法
            if (controller.transactionViewModel) {
                // 使用新添加的更可靠的方法
                controller.transactionViewModel.updateCardNumber(cardNum)
                controller.transactionViewModel.setRecentTransactionCount(20) // 显示最近20条交易
                
                // 强制刷新
                controller.transactionViewModel.refreshTransactions()
                
                // 更新调试标签
                var count = controller.transactionViewModel.rowCount()
                debugLabel.text = "调试: 已加载 " + count + " 条交易记录"
                
                // 打印调试信息
                console.log("刷新交易历史: 卡号=" + cardNum + ", 记录数=" + count)
            } else {
                debugLabel.text = "调试: TransactionViewModel不可用"
                console.log("TransactionViewModel不可用")
            }
        } else {
            debugLabel.text = "调试: 未登录或卡号无效，无法刷新交易历史"
            console.log("未登录或卡号无效，无法刷新交易历史")
        }
    }
    
    // 每次页面变为可见时刷新交易记录
    onVisibleChanged: {
        if (visible) {
            console.log("交易历史页面变为可见，即将刷新数据")
            Qt.callLater(refreshTransactionHistory)
        }
    }
    
    // 使用ScrollView确保在小屏幕上可以滚动
    ScrollView {
        anchors {
            top: debugLabel.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: 10
        }
        contentWidth: parent.width
        contentHeight: contentColumn.height
        clip: true
        
        ColumnLayout {
            id: contentColumn
            width: parent.width
            spacing: 15
            
            // 账户信息
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 10
                height: 60
                color: "#2a2d3a"
                radius: 5
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Label {
                        text: "持卡人: " + controller.accountViewModel.holderName
                        font.pixelSize: 16
                    }
                    
                    Rectangle {
                        width: 1
                        height: parent.height * 0.7
                        color: "#555"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    
                    Label {
                        text: "当前余额: ￥" + controller.accountViewModel.balance.toFixed(2)
                        font.pixelSize: 16
                        font.bold: true
                        Layout.alignment: Qt.AlignRight
                        Layout.fillWidth: true
                    }
                }
            }
            
            // 标题栏
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                height: 40
                color: Material.color(Material.Blue, Material.Shade700)
                radius: 5
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    Label {
                        text: "交易类型"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                        Layout.preferredWidth: 80
                    }
                    
                    Label {
                        text: "交易时间"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                        Layout.preferredWidth: 180
                    }
                    
                    Label {
                        text: "交易金额"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                        Layout.preferredWidth: 100
                        horizontalAlignment: Text.AlignRight
                    }
                    
                    Label {
                        text: "交易后余额"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                        Layout.preferredWidth: 100
                        horizontalAlignment: Text.AlignRight
                    }
                    
                    Label {
                        text: "交易描述"
                        font.pixelSize: 14
                        font.bold: true
                        color: "white"
                        Layout.fillWidth: true
                    }
                }
            }
            
            // 交易列表容器
            Rectangle {
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.preferredHeight: 350
                color: "transparent"
                
                // 交易列表
                ListView {
                    id: transactionsList
                    anchors.fill: parent
                    clip: true
                    model: controller.transactionViewModel
                    
                    // 没有交易记录时显示
                    Label {
                        visible: transactionsList.count === 0
                        anchors.centerIn: parent
                        text: "暂无交易记录"
                        font.pixelSize: 18
                        color: Material.color(Material.Grey)
                    }
                    
                    delegate: Rectangle {
                        width: transactionsList.width
                        height: 60
                        color: index % 2 === 0 ? "#1e2029" : "#2a2d3a"
                        
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10
                            
                            // 交易类型
                            Label {
                                text: controller.transactionViewModel.getTransactionTypeName(type)
                                font.pixelSize: 14
                                color: {
                                    switch(type) {
                                        case 0: return "#4caf50"; // 存款-绿色
                                        case 1: return "#f44336"; // 取款-红色
                                        case 2: return "#2196f3"; // 余额查询-蓝色
                                        case 3: return "#ff9800"; // 转账-橙色
                                        default: return "white";
                                    }
                                }
                                Layout.preferredWidth: 80
                            }
                            
                            // 交易时间
                            Label {
                                text: controller.transactionViewModel.formatDate(timestamp)
                                font.pixelSize: 14
                                color: "white"
                                Layout.preferredWidth: 180
                            }
                            
                            // 交易金额
                            Label {
                                text: {
                                    if (type === 0) return "+￥" + controller.transactionViewModel.formatAmount(amount);
                                    else if (type === 1 || type === 3) return "-￥" + controller.transactionViewModel.formatAmount(amount);
                                    else return "￥0.00";
                                }
                                font.pixelSize: 14
                                color: {
                                    if (type === 0) return "#4caf50"; // 存款-绿色
                                    else if (type === 1 || type === 3) return "#f44336"; // 取款/转账-红色
                                    else return "white";
                                }
                                Layout.preferredWidth: 100
                                horizontalAlignment: Text.AlignRight
                            }
                            
                            // 交易后余额
                            Label {
                                text: "￥" + controller.transactionViewModel.formatAmount(balanceAfter)
                                font.pixelSize: 14
                                color: "white"
                                Layout.preferredWidth: 100
                                horizontalAlignment: Text.AlignRight
                            }
                            
                            // 交易描述
                            Label {
                                text: description
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                color: "white"
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }
            
            // 底部留白
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 30
            }
        }
    }
}