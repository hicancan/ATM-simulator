import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    id: page
    
    // 添加ReceiptPrinter组件
    ReceiptPrinter {
        id: receiptPrinter
        visible: false // 组件不需要可见
    }
    
    background: Rectangle {
        color: "#1e2029"
    }
    
    // Header
    HeaderBar {
        id: header
        title: "取款"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: controller.switchToPage("MainMenu")
    }
    
    // 添加滚动视图
    ScrollView {
        id: scrollView
        anchors {
            top: header.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        contentWidth: parent.width
        clip: true
        
        ColumnLayout {
            width: scrollView.width
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 20
            spacing: 15
            
            // Account info
            Rectangle {
                Layout.fillWidth: true
                height: 60
                color: "#2a2d3a"
                radius: 5
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Label {
                        text: "账户余额: ￥" + controller.accountViewModel.balance.toFixed(2)
                        font.pixelSize: 18
                        Layout.fillWidth: true
                    }
                    
                    Label {
                        text: "取款限额: ￥" + controller.accountViewModel.withdrawLimit.toFixed(2)
                        font.pixelSize: 18
                    }
                }
            }
            
            // Quick Withdraw Options
            Label {
                text: "快速取款:"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
            }
            
            GridLayout {
                Layout.alignment: Qt.AlignHCenter
                columns: 3
                rows: 2
                columnSpacing: 15
                rowSpacing: 15
                
                Repeater {
                    model: [100, 500, 1000, 2000, 3000, 5000]
                    
                    Button {
                        text: "￥" + modelData
                        Layout.preferredWidth: 130
                        Layout.preferredHeight: 50
                        font.pixelSize: 16
                        enabled: modelData <= controller.accountViewModel.withdrawLimit && 
                                 modelData <= controller.accountViewModel.balance
                        
                        onClicked: {
                            if (controller.accountViewModel.withdraw(modelData)) {
                                resultDialog.withdrewAmount = modelData
                                resultDialog.success = true
                                resultDialog.open()
                            }
                        }
                    }
                }
            }
            
            // Custom amount
            Label {
                text: "自定义金额:"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 15
            }
            
            // 数字键盘和按钮垂直布局
            ColumnLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 15
                
                // 数字键盘
                NumericKeypad {
                    id: keypad
                    isAmount: true
                    maxValue: Math.min(controller.accountViewModel.withdrawLimit, 
                                      controller.accountViewModel.balance)
                    Layout.alignment: Qt.AlignHCenter
                }
                
                // 操作按钮
                RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 15
                    
                    Button {
                        id: confirmButton
                        text: "确认取款"
                        Layout.preferredWidth: 130
                        Layout.preferredHeight: 50
                        font.pixelSize: 16
                        Material.background: Material.Green
                        
                        onClicked: {
                            var amount = parseFloat(keypad.displayText)
                            if (!isNaN(amount) && amount > 0) {
                                if (controller.accountViewModel.withdraw(amount)) {
                                    resultDialog.withdrewAmount = amount
                                    resultDialog.success = true
                                    keypad.clear()
                                    resultDialog.open()
                                }
                            }
                        }
                    }
                    
                    Button {
                        text: "返回主菜单"
                        Layout.preferredWidth: 130
                        Layout.preferredHeight: 50
                        font.pixelSize: 16
                        Material.background: Material.Blue
                        
                        onClicked: controller.switchToPage("MainMenu")
                    }
                }
            }
            
            // Show error message if any
            Label {
                id: errorLabel
                text: controller.accountViewModel.errorMessage
                color: "red"
                visible: text !== ""
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 16
                Layout.bottomMargin: 15
            }
            
            // 底部空白，确保滚动时有足够的空间
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }
        }
    }
    
    // Transaction Result Dialog
    Dialog {
        id: resultDialog
        title: resultDialog.success ? "交易成功" : "交易失败"
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 400
        
        property bool success: false
        property real withdrewAmount: 0
        
        contentItem: ColumnLayout {
            spacing: 20
            
            Label {
                text: resultDialog.success ? 
                    "您已成功取款：￥" + resultDialog.withdrewAmount.toFixed(2) : 
                    "交易失败，请重试"
                font.pixelSize: 16
                Layout.fillWidth: true
                wrapMode: Label.Wrap
            }
            
            Label {
                text: "当前余额：￥" + controller.accountViewModel.balance.toFixed(2)
                font.pixelSize: 16
                visible: resultDialog.success
            }
        }
        
        footer: DialogButtonBox {
            Button {
                text: "打印回单"
                visible: resultDialog.success
                DialogButtonBox.buttonRole: DialogButtonBox.HelpRole
                onClicked: {
                    receiptPrinter.printWithdrawalReceipt(
                        controller.accountViewModel.cardNumber,
                        controller.accountViewModel.holderName,
                        resultDialog.withdrewAmount,
                        controller.accountViewModel.balance
                    )
                }
            }
            
            Button {
                text: "继续取款"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            
            Button {
                text: "返回主菜单"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                onClicked: controller.switchToPage("MainMenu")
            }
        }
    }
}