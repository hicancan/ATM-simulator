import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtQuick.Window 2.15

Item {
    id: root
    
    // 打印回单属性
    property string bankName: "ATM 模拟器银行"
    property string cardNumber: ""
    property string holderName: ""
    property string transactionType: ""  // 取款，存款，转账
    property real amount: 0
    property real balanceAfter: 0
    property string targetCardNumber: ""
    property string targetCardHolder: ""
    property date transactionDate: new Date()
    property string transactionId: generateTransactionId()
    
    // 生成交易ID
    function generateTransactionId() {
        var chars = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        var result = "";
        for (var i = 0; i < 10; i++) {
            result += chars.charAt(Math.floor(Math.random() * chars.length));
        }
        return result;
    }
    
    // 打印存款回单
    function printDepositReceipt(cardNum, holder, depositAmount, balance) {
        cardNumber = cardNum;
        holderName = holder;
        transactionType = "存款";
        amount = depositAmount;
        balanceAfter = balance;
        transactionDate = new Date();
        transactionId = generateTransactionId();
        
        receiptDialog.open();
    }
    
    // 打印取款回单
    function printWithdrawalReceipt(cardNum, holder, withdrawAmount, balance) {
        cardNumber = cardNum;
        holderName = holder;
        transactionType = "取款";
        amount = withdrawAmount;
        balanceAfter = balance;
        transactionDate = new Date();
        transactionId = generateTransactionId();
        
        receiptDialog.open();
    }
    
    // 打印转账回单
    function printTransferReceipt(cardNum, holder, transferAmount, balance, targetCard, targetHolder) {
        cardNumber = cardNum;
        holderName = holder;
        transactionType = "转账";
        amount = transferAmount;
        balanceAfter = balance;
        targetCardNumber = targetCard;
        targetCardHolder = targetHolder;
        transactionDate = new Date();
        transactionId = generateTransactionId();
        
        receiptDialog.open();
    }
    
    // 格式化日期时间
    function formatDateTime(date) {
        function pad(n) { return n < 10 ? '0' + n : n }
        
        return date.getFullYear() + '-' + 
               pad(date.getMonth() + 1) + '-' + 
               pad(date.getDate()) + ' ' + 
               pad(date.getHours()) + ':' + 
               pad(date.getMinutes()) + ':' + 
               pad(date.getSeconds());
    }
    
    // 回单对话框
    Dialog {
        id: receiptDialog
        title: "交易回单"
        modal: true
        anchors.centerIn: Overlay.overlay
        width: 500
        height: 600
        
        contentItem: ScrollView {
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            
            ColumnLayout {
                width: parent.parent.width - 20
                spacing: 10
                
                // 回单标题
                Label {
                    text: root.bankName
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 2
                    color: "black"
                }
                
                // 回单内容
                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    rowSpacing: 10
                    columnSpacing: 10
                    
                    Label { text: "交易类型:"; font.pixelSize: 14; font.bold: true }
                    Label { text: root.transactionType; font.pixelSize: 14 }
                    
                    Label { text: "交易时间:"; font.pixelSize: 14; font.bold: true }
                    Label { text: formatDateTime(root.transactionDate); font.pixelSize: 14 }
                    
                    Label { text: "交易卡号:"; font.pixelSize: 14; font.bold: true }
                    Label { text: "尾号" + root.cardNumber.slice(-4); font.pixelSize: 14 }
                    
                    Label { text: "持卡人:"; font.pixelSize: 14; font.bold: true }
                    Label { text: root.holderName; font.pixelSize: 14 }
                    
                    Label { text: "交易金额:"; font.pixelSize: 14; font.bold: true }
                    Label { text: "￥" + root.amount.toFixed(2); font.pixelSize: 14 }
                    
                    Label { 
                        text: "交易后余额:"; 
                        font.pixelSize: 14; 
                        font.bold: true 
                    }
                    Label { 
                        text: "￥" + root.balanceAfter.toFixed(2); 
                        font.pixelSize: 14 
                    }
                    
                    // 如果是转账，显示目标账户信息
                    Label { 
                        text: "收款卡号:"; 
                        font.pixelSize: 14; 
                        font.bold: true;
                        visible: root.transactionType === "转账"
                    }
                    Label { 
                        text: "尾号" + root.targetCardNumber.slice(-4); 
                        font.pixelSize: 14;
                        visible: root.transactionType === "转账"
                    }
                    
                    Label { 
                        text: "收款人:"; 
                        font.pixelSize: 14; 
                        font.bold: true;
                        visible: root.transactionType === "转账"
                    }
                    Label { 
                        text: root.targetCardHolder; 
                        font.pixelSize: 14;
                        visible: root.transactionType === "转账"
                    }
                    
                    Label { text: "交易编号:"; font.pixelSize: 14; font.bold: true }
                    Label { text: root.transactionId; font.pixelSize: 14 }
                }
                
                Rectangle {
                    Layout.fillWidth: true
                    height: 2
                    color: "black"
                    Layout.topMargin: 10
                }
                
                // 回单说明
                Label {
                    text: "此回单作为交易凭证，请妥善保管。\n感谢您使用ATM模拟器银行服务！"
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                    Layout.topMargin: 10
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
        
        footer: DialogButtonBox {
            Button {
                text: "打印回单"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                onClicked: {
                    // 在真实系统中这里会调用打印机接口
                    // 这里只是模拟打印过程
                    printSuccessDialog.open();
                }
            }
            
            Button {
                text: "关闭"
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }
    }
    
    // 打印成功对话框
    Dialog {
        id: printSuccessDialog
        title: "打印成功"
        modal: true
        anchors.centerIn: Overlay.overlay
        
        Label {
            text: "回单已成功打印，请取走您的回单。"
            anchors.centerIn: parent
            padding: 20
        }
        
        footer: DialogButtonBox {
            Button {
                text: "确定"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
    }
} 