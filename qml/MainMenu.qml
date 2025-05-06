import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    background: Rectangle {
        color: "#1e2029"
    }
    
    HeaderBar {
        id: header
        title: "主菜单 - 欢迎, " + controller.accountViewModel.holderName
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }
    
    // User info panel
    Rectangle {
        id: userInfoPanel
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 80
        color: "#2a2d3a"
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 20
            
            Label {
                text: "卡号: " + controller.accountViewModel.cardNumber
                font.pixelSize: 14
                Layout.alignment: Qt.AlignLeft
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
    
    // Main menu options
    GridLayout {
        anchors {
            top: userInfoPanel.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: 20
        }
        
        columns: 2
        rows: 3
        rowSpacing: 20
        columnSpacing: 30
        
        TransactionButton {
            buttonText: "查询余额"
            Layout.alignment: Qt.AlignHCenter
            
            onClicked: controller.switchToPage("BalancePage")
        }
        
        TransactionButton {
            buttonText: "存款"
            Layout.alignment: Qt.AlignHCenter
            
            onClicked: controller.switchToPage("DepositPage")
        }
        
        TransactionButton {
            buttonText: "取款"
            Layout.alignment: Qt.AlignHCenter
            
            onClicked: controller.switchToPage("WithdrawPage")
        }
        
        TransactionButton {
            buttonText: "转账"
            Layout.alignment: Qt.AlignHCenter
            
            onClicked: controller.switchToPage("TransferPage")
        }
        
        TransactionButton {
            buttonText: "修改密码"
            Layout.alignment: Qt.AlignHCenter
            Layout.row: 2
            Layout.column: 0
            
            onClicked: controller.switchToPage("ChangePasswordPage")
        }
        
        TransactionButton {
            buttonText: "退出系统"
            Layout.alignment: Qt.AlignHCenter
            Layout.row: 2
            Layout.column: 1
            
            onClicked: {
                controller.logout()
            }
        }
    }
} 