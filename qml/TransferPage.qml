import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    id: page
    
    // Clear error message when page is loaded
    Component.onCompleted: {
        controller.accountViewModel.clearError()
        keypad.clear()
        targetCardField.text = ""
    }
    
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
        title: "转账"
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
                }
            }
            
            // 输入转账卡号
            Label {
                text: "请输入转账目标卡号:"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 10
            }
            
            TextField {
                id: targetCardField
                placeholderText: "16位银行卡号"
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 16
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: TextInput.AlignHCenter
                font.pixelSize: 18
                
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
                color: "white"
                
                onTextChanged: {
                    controller.accountViewModel.clearError()
                    if (text.length === 16) {
                        var holderName = controller.accountViewModel.getTargetCardHolderName(text)
                        if (holderName && holderName.length > 0) {
                            receiverInfo.text = "接收方: " + holderName + " (卡号尾号" + text.slice(-4) + ")"
                            receiverInfo.visible = true
                        } else {
                            receiverInfo.visible = false
                        }
                    } else {
                        receiverInfo.visible = false
                    }
                }
            }
            
            // 显示接收方信息
            Label {
                id: receiverInfo
                text: ""
                font.pixelSize: 16
                color: Material.color(Material.Green)
                Layout.alignment: Qt.AlignHCenter
                visible: false
            }
            
            // 输入转账金额
            Label {
                text: "请输入转账金额:"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 15
            }
            
            // 快速转账选项
            Label {
                text: "快速转账:"
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
                    model: [100, 500, 1000, 5000, 10000, 50000]
                    
                    Button {
                        text: "￥" + modelData
                        Layout.preferredWidth: 130
                        Layout.preferredHeight: 50
                        font.pixelSize: 16
                        
                        onClicked: {
                            keypad.displayText = modelData.toString()
                        }
                    }
                }
            }
            
            // 自定义金额
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
                    // 不再限制最大金额
                    maxValue: Number.MAX_VALUE // 设置为极大值，实际上不再使用
                    Layout.alignment: Qt.AlignHCenter
                }
                
                // 操作按钮
                BottomActionButtons {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.topMargin: 10
                    confirmText: "确认转账"
                    cancelText: "返回主菜单"
                    
                    onConfirmed: {
                        if (targetCardField.text.length !== 16) {
                            controller.accountViewModel.setErrorMessage("请输入16位目标卡号")
                            return
                        }
                        
                        var amount = parseFloat(keypad.displayText)
                        if (isNaN(amount) || amount <= 0) {
                            controller.accountViewModel.setErrorMessage("请输入有效金额")
                            return
                        }
                        
                        // 显示确认对话框
                        confirmDialog.targetCard = targetCardField.text
                        confirmDialog.transferAmount = amount
                        confirmDialog.open()
                    }
                    
                    onCanceled: controller.switchToPage("MainMenu")
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
                
                // 添加背景以使错误消息更加明显
                background: Rectangle {
                    visible: errorLabel.text !== ""
                    color: "#44FF0000"  // 半透明红色背景
                    radius: 5
                    border.color: "red"
                    border.width: 1
                    implicitWidth: errorLabel.implicitWidth + 20
                    implicitHeight: errorLabel.implicitHeight + 10
                }
                
                padding: 5  // 为文本添加内边距
            }
            
            // 底部空白，确保滚动时有足够的空间
            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: 20
            }
        }
    }
    
    // 转账确认对话框
    Dialog {
        id: confirmDialog
        title: "确认转账"
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 400
        
        property string targetCard: ""
        property real transferAmount: 0
        
        // Clear error message when dialog is closed
        onClosed: {
            controller.accountViewModel.clearError()
        }
        
        contentItem: ColumnLayout {
            spacing: 20
            
            Label {
                text: "您将要转账 ￥" + confirmDialog.transferAmount.toFixed(2) + 
                      " 至 " + controller.accountViewModel.getTargetCardHolderName(confirmDialog.targetCard) + 
                      "（卡号尾号" + confirmDialog.targetCard.slice(-4) + "）"
                font.pixelSize: 16
                Layout.fillWidth: true
                wrapMode: Label.Wrap
            }
            
            // 添加余额警告提示
            Label {
                text: "当前账户余额: ￥" + controller.accountViewModel.balance.toFixed(2)
                font.pixelSize: 14
                Layout.fillWidth: true
            }
            
            // 警告标签 - 如果转账金额大于余额则显示
            Label {
                text: "警告: 转账金额超出您的当前余额!"
                font.pixelSize: 14
                font.bold: true
                color: "red"
                visible: confirmDialog.transferAmount > controller.accountViewModel.balance
                Layout.fillWidth: true
                
                background: Rectangle {
                    visible: confirmDialog.transferAmount > controller.accountViewModel.balance
                    color: "#22FF0000"  // 半透明红色背景
                    radius: 3
                    implicitWidth: parent.width
                    implicitHeight: parent.height + 6
                }
                
                padding: 3
            }
            
            Label {
                text: "确认后将从您的账户扣除该金额。"
                font.pixelSize: 14
                Layout.fillWidth: true
                wrapMode: Label.Wrap
            }
        }
        
        footer: DialogButtonBox {
            Button {
                text: "确认转账"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            
            Button {
                text: "取消"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }
        
        onAccepted: {
            if (controller.accountViewModel.transfer(targetCard, transferAmount)) {
                resultDialog.transferAmount = transferAmount
                resultDialog.targetCard = targetCard
                resultDialog.success = true
                keypad.clear()
                targetCardField.text = ""
                resultDialog.open()
            } else {
                // 如果转账失败，显示结果对话框并传递失败状态
                resultDialog.transferAmount = transferAmount
                resultDialog.targetCard = targetCard
                resultDialog.success = false
                resultDialog.open()
            }
        }
    }
    
    // 交易结果对话框
    Dialog {
        id: resultDialog
        title: resultDialog.success ? "交易成功" : "交易失败"
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 400
        
        property bool success: false
        property real transferAmount: 0
        property string targetCard: ""
        
        // Clear error message when dialog is closed
        onClosed: {
            controller.accountViewModel.clearError()
            keypad.clear()
        }
        
        contentItem: ColumnLayout {
            spacing: 20
            
            Label {
                text: resultDialog.success ? 
                    "已成功转账：￥" + resultDialog.transferAmount.toFixed(2) + 
                    " 至 " + controller.accountViewModel.getTargetCardHolderName(resultDialog.targetCard) : 
                    "转账失败：" + controller.accountViewModel.errorMessage
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
                    receiptPrinter.printTransferReceipt(
                        controller.accountViewModel.cardNumber,
                        controller.accountViewModel.holderName,
                        resultDialog.transferAmount,
                        controller.accountViewModel.balance,
                        resultDialog.targetCard,
                        controller.accountViewModel.getTargetCardHolderName(resultDialog.targetCard)
                    )
                }
            }
            
            Button {
                text: "继续转账"
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