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
        title: "余额查询"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: controller.switchToPage("MainMenu")
    }
    
    // 使用ScrollView允许在小屏幕上滚动
    ScrollView {
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        contentWidth: parent.width
        contentHeight: contentColumn.height
        clip: true
        
        ColumnLayout {
            id: contentColumn
            width: parent.width
            spacing: 20
            
            // 顶部边距
            Item {
                Layout.fillWidth: true
                height: 10
            }
            
            // Account Info Panel
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)  // 响应式宽度
                Layout.preferredHeight: 280  // 减小高度
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                color: Material.color(Material.BlueGrey, Material.Shade800)
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 20  // 减少间距
                    
                    Label {
                        text: "账户详情"
                        font.pixelSize: 22  // 减小字体
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                        color: "white"
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Material.color(Material.Grey, Material.Shade400)
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "持卡人: "
                            font.pixelSize: 16  // 减小字体
                            color: "white"
                        }
                        
                        Label {
                            text: controller.accountViewModel.holderName
                            font.pixelSize: 16  // 减小字体
                            font.bold: true
                            color: "white"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "卡号: "
                            font.pixelSize: 16  // 减小字体
                            color: "white"
                        }
                        
                        Label {
                            text: controller.accountViewModel.cardNumber
                            font.pixelSize: 16  // 减小字体
                            font.bold: true
                            color: "white"
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "可用余额: "
                            font.pixelSize: 16  // 减小字体
                            color: "white"
                        }
                        
                        Label {
                            text: "￥" + controller.accountViewModel.balance.toFixed(2)
                            font.pixelSize: 22  // 减小字体
                            font.bold: true
                            color: "#4caf50"
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                    
                    RowLayout {
                        Layout.fillWidth: true
                        
                        Label {
                            text: "取款限额: "
                            font.pixelSize: 16  // 减小字体
                            color: "white"
                        }
                        
                        Label {
                            text: "￥" + controller.accountViewModel.withdrawLimit.toFixed(2)
                            font.pixelSize: 16  // 减小字体
                            font.bold: true
                            color: "#ff9800"
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignRight
                        }
                    }
                }
            }
            
            // 按钮使用FlowLayout，在小屏幕上自动换行
            Flow {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                spacing: 20
                
                Button {
                    text: "取款"
                    width: 120
                    height: 50
                    font.pixelSize: 16
                    
                    onClicked: controller.switchToPage("WithdrawPage")
                }
                
                Button {
                    text: "存款"
                    width: 120
                    height: 50
                    font.pixelSize: 16
                    
                    onClicked: controller.switchToPage("DepositPage")
                }
                
                Button {
                    text: "交易历史"
                    width: 120
                    height: 50
                    font.pixelSize: 16
                    Material.background: Material.Purple
                    
                    onClicked: controller.switchToPage("TransactionHistoryPage")
                }
                
                Button {
                    text: "返回主菜单"
                    width: 120
                    height: 50
                    font.pixelSize: 16
                    
                    onClicked: controller.switchToPage("MainMenu")
                }
            }
            
            // 底部边距
            Item {
                Layout.fillWidth: true
                height: 20
            }
        }
    }
} 