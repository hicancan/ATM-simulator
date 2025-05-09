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
        title: "余额预测"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: controller.switchToPage("BalancePage")
    }
    
    // 使用ScrollView允许在小屏幕上滚动
    ScrollView {
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            topMargin: 10
        }
        contentWidth: parent.width
        clip: true
        
        ColumnLayout {
            id: contentColumn
            width: parent.width
            spacing: 30
            Layout.fillWidth: true
            
            // 顶部边距
            Item {
                Layout.fillWidth: true
                height: 20
            }
            
            // 余额预测面板
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                color: Material.color(Material.Teal, Material.Shade800)
                implicitHeight: predictionColumn.implicitHeight + 40

                ColumnLayout {
                    id: predictionColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Label {
                        text: "余额预测 (未来7天)"
                        font.pixelSize: 20
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                        color: "white"
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Material.color(Material.Grey, Material.Shade400)
                    }

                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 10
                        
                        Label { 
                            text: "预测余额:"
                            font.pixelSize: 16
                            color: "white"
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        }
                        
                        Label {
                            id: predictedBalanceLabel
                            text: controller.accountViewModel.predictedBalance > 0 || controller.accountViewModel.predictedBalance < 0 ?
                                  "￥" + controller.accountViewModel.predictedBalance.toFixed(2) :
                                  (controller.accountViewModel.isLoggedIn ? "点击下方按钮计算" : "N/A")
                            font.pixelSize: 18
                            font.bold: true
                            color: controller.accountViewModel.predictedBalance > controller.accountViewModel.balance ? 
                                   "#4caf50" : (controller.accountViewModel.predictedBalance < controller.accountViewModel.balance ? 
                                   "#f44336" : "white")
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                    
                    Item {
                        Layout.fillWidth: true
                        height: 10
                    }
                    
                    Button {
                        text: "计算预测余额"
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: 40
                        font.pixelSize: 14
                        Material.background: Material.Orange
                        enabled: controller.accountViewModel.isLoggedIn
                        onClicked: {
                            controller.accountViewModel.calculatePredictedBalance(7) 
                        }
                    }
                }
            }
            
            // 操作按钮区域
            GridLayout {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                Layout.topMargin: 20
                Layout.bottomMargin: 30
                columns: width < 400 ? 1 : 2 // 窄屏幕使用1列，宽屏幕使用2列
                rowSpacing: 15
                columnSpacing: 15

                Repeater {
                    model: [
                        { "text": "取款", "page": "WithdrawPage", "color": Material.Green },
                        { "text": "存款", "page": "DepositPage", "color": Material.Blue },
                        { "text": "交易历史", "page": "TransactionHistoryPage", "color": Material.Purple },
                        { "text": "返回主菜单", "page": "MainMenu", "color": Material.Grey }
                    ]

                    Button {
                        text: modelData.text
                        Layout.fillWidth: true
                        Layout.preferredHeight: 50
                        font.pixelSize: 16
                        Material.background: Material.color(modelData.color)
                        
                        onClicked: controller.switchToPage(modelData.page)
                    }
                }
            }
            
            // 底部空间
            Item {
                Layout.fillWidth: true
                height: 20
            }
        }
    }
} 