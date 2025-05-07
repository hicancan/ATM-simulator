import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    id: adminPage
    
    property var selectedAccount: null // 存储选中的账户信息
    
    background: Rectangle {
        color: "#1e2029"
    }
    
    // Header
    HeaderBar {
        id: header
        title: "管理员控制面板"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: {
            controller.logout()
        }
    }
    
    // Main content
    ScrollView {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        contentWidth: parent.width
        clip: true
        
        ColumnLayout {
            width: parent.width
            spacing: 20
            
            // 警告标志
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 10
                height: 40
                color: "#f44336"
                radius: 5
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Label {
                        text: "\u26a0" // 警告符号
                        font.pixelSize: 18
                        color: "white"
                    }
                    
                    Label {
                        text: "管理员模式 - 请谨慎操作"
                        font.pixelSize: 16
                        font.bold: true
                        color: "white"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
            
            // 账户管理部分
            Rectangle {
                Layout.fillWidth: true
                Layout.margins: 10
                Layout.preferredHeight: 400
                color: Material.color(Material.BlueGrey, Material.Shade800)
                radius: 5
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 10
                    
                    Label {
                        text: "账户管理"
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        Layout.alignment: Qt.AlignHCenter
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Material.color(Material.Grey, Material.Shade400)
                    }
                    
                    // 账户列表与详情的布局
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 10
                        
                        // 账户列表
                        Rectangle {
                            Layout.fillHeight: true
                            Layout.preferredWidth: parent.width * 0.4
                            color: "#2a2d3a"
                            radius: 5
                            
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 5
                                spacing: 5
                                
                                Label {
                                    text: "账户列表"
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: "white"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 1
                                    color: "#555"
                                }
                                
                                // 账户列表
                                ListView {
                                    id: accountListView
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    clip: true
                                    model: controller.accountViewModel.getAllAccounts()
                                    
                                    Component.onCompleted: {
                                        if (model.length > 0) {
                                            currentIndex = 0
                                            selectedAccount = model[0]
                                        }
                                    }
                                    
                                    onCurrentIndexChanged: {
                                        if (currentIndex >= 0 && currentIndex < model.length) {
                                            selectedAccount = model[currentIndex]
                                        }
                                    }
                                    
                                    delegate: ItemDelegate {
                                        width: parent ? parent.width : 0
                                        height: 50
                                        
                                        // 高亮选中项
                                        highlighted: ListView.isCurrentItem
                                        
                                        onClicked: {
                                            accountListView.currentIndex = index
                                        }
                                        
                                        contentItem: RowLayout {
                                            spacing: 5
                                            
                                            // 管理员标记
                                            Rectangle {
                                                visible: modelData.isAdmin
                                                width: 12
                                                height: 12
                                                radius: 6
                                                color: "#f44336"
                                            }
                                            
                                            // 锁定标记
                                            Rectangle {
                                                visible: modelData.isLocked
                                                width: 12
                                                height: 12
                                                radius: 6
                                                color: "#ff9800"
                                            }
                                            
                                            Label {
                                                text: modelData.holderName
                                                color: "white"
                                                font.bold: true
                                                Layout.fillWidth: true
                                                elide: Text.ElideRight
                                            }
                                            
                                            Label {
                                                text: modelData.cardNumber.substring(12)
                                                color: "#aaa"
                                                font.pixelSize: 12
                                            }
                                        }
                                    }
                                    
                                    ScrollBar.vertical: ScrollBar {}
                                }
                                
                                // 刷新按钮
                                Button {
                                    text: "刷新列表"
                                    Layout.fillWidth: true
                                    Layout.preferredHeight: 40
                                    
                                    onClicked: {
                                        accountListView.model = controller.accountViewModel.getAllAccounts()
                                    }
                                }
                            }
                        }
                        
                        // 账户详情和操作
                        Rectangle {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            color: "#2a2d3a"
                            radius: 5
                            
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                
                                Label {
                                    text: selectedAccount ? "账户详情: " + selectedAccount.holderName : "请选择账户"
                                    font.pixelSize: 16
                                    font.bold: true
                                    color: "white"
                                    Layout.alignment: Qt.AlignHCenter
                                }
                                
                                GridLayout {
                                    visible: selectedAccount !== null
                                    Layout.fillWidth: true
                                    columns: 2
                                    columnSpacing: 10
                                    rowSpacing: 5
                                    
                                    Label { text: "卡号:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? selectedAccount.cardNumber : ""
                                        color: "white"; font.bold: true; Layout.fillWidth: true 
                                    }
                                    
                                    Label { text: "持卡人:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? selectedAccount.holderName : ""
                                        color: "white"; font.bold: true; Layout.fillWidth: true 
                                    }
                                    
                                    Label { text: "余额:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? "￥" + selectedAccount.balance.toFixed(2) : ""
                                        color: "#4caf50"; font.bold: true; Layout.fillWidth: true 
                                    }
                                    
                                    Label { text: "取款限额:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? "￥" + selectedAccount.withdrawLimit.toFixed(2) : ""
                                        color: "#ff9800"; font.bold: true; Layout.fillWidth: true 
                                    }
                                    
                                    Label { text: "账户状态:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? (selectedAccount.isLocked ? "已锁定" : "正常") : ""
                                        color: selectedAccount && selectedAccount.isLocked ? "#f44336" : "#4caf50"
                                        font.bold: true; Layout.fillWidth: true 
                                    }
                                    
                                    Label { text: "账户类型:"; color: "white" }
                                    Label { 
                                        text: selectedAccount ? (selectedAccount.isAdmin ? "管理员" : "普通用户") : ""
                                        color: selectedAccount && selectedAccount.isAdmin ? "#f44336" : "white"
                                        font.bold: true; Layout.fillWidth: true 
                                    }
                                }
                                
                                Item { Layout.fillHeight: true }
                                
                                // 账户操作按钮
                                GridLayout {
                                    visible: selectedAccount !== null
                                    Layout.fillWidth: true
                                    columns: 2
                                    rowSpacing: 10
                                    columnSpacing: 10
                                    
                                    Button {
                                        text: selectedAccount && selectedAccount.isLocked ? "解锁账户" : "锁定账户"
                                        Layout.fillWidth: true
                                        enabled: selectedAccount && selectedAccount.cardNumber !== controller.accountViewModel.cardNumber
                                        Material.background: selectedAccount && selectedAccount.isLocked ? 
                                                             Material.Green : Material.Red
                                        
                                        onClicked: {
                                            if (selectedAccount) {
                                                var newLockedState = !selectedAccount.isLocked
                                                if (controller.accountViewModel.setAccountLockStatus(selectedAccount.cardNumber, newLockedState)) {
                                                    accountListView.model = controller.accountViewModel.getAllAccounts()
                                                    // 重新选择相同的账户
                                                    var newIndex = findAccountIndex(selectedAccount.cardNumber)
                                                    if (newIndex >= 0) {
                                                        accountListView.currentIndex = newIndex
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    
                                    Button {
                                        text: "重置PIN码"
                                        Layout.fillWidth: true
                                        Material.background: Material.Blue
                                        
                                        onClicked: resetPinDialog.open()
                                    }
                                    
                                    Button {
                                        text: "修改限额"
                                        Layout.fillWidth: true
                                        Material.background: Material.Orange
                                        
                                        onClicked: limitDialog.open()
                                    }
                                    
                                    Button {
                                        text: "删除账户"
                                        Layout.fillWidth: true
                                        Material.background: Material.Red
                                        enabled: selectedAccount && selectedAccount.cardNumber !== controller.accountViewModel.cardNumber
                                        
                                        onClicked: deleteDialog.open()
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // 创建新账户
            Button {
                text: "创建新账户"
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                Material.background: Material.Green
                
                onClicked: newAccountDialog.open()
            }
            
            // 返回按钮
            Button {
                text: "注销"
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                Material.background: Material.Red
                
                onClicked: controller.logout()
            }
            
            // 底部空间
            Item {
                Layout.fillWidth: true
                height: 20
            }
        }
    }
    
    // 查找账户索引的辅助函数
    function findAccountIndex(cardNumber) {
        var accounts = controller.accountViewModel.getAllAccounts()
        for (var i = 0; i < accounts.length; i++) {
            if (accounts[i].cardNumber === cardNumber) {
                return i
            }
        }
        return -1
    }
    
    // 重置PIN对话框
    Dialog {
        id: resetPinDialog
        title: "重置PIN码"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width - 50, 400)
        
        ColumnLayout {
            spacing: 20
            width: parent.width
            
            Label {
                text: "为账户 " + (selectedAccount ? selectedAccount.holderName : "") + " 设置新的PIN码"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            TextField {
                id: newPinField
                placeholderText: "新PIN码 (4位数字)"
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 4
                
                validator: RegularExpressionValidator {
                    regularExpression: /^[0-9]{0,4}$/
                }
            }
        }
        
        onAccepted: {
            if (selectedAccount && newPinField.text.length === 4) {
                controller.accountViewModel.resetAccountPin(selectedAccount.cardNumber, newPinField.text)
                newPinField.text = ""
            }
        }
        
        onRejected: {
            newPinField.text = ""
        }
    }
    
    // 修改限额对话框
    Dialog {
        id: limitDialog
        title: "修改取款限额"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width - 50, 400)
        
        ColumnLayout {
            spacing: 20
            width: parent.width
            
            Label {
                text: "为账户 " + (selectedAccount ? selectedAccount.holderName : "") + " 设置新的取款限额"
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            TextField {
                id: newLimitField
                placeholderText: "新限额"
                text: selectedAccount ? selectedAccount.withdrawLimit.toString() : ""
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                
                validator: DoubleValidator {
                    bottom: 0
                    notation: DoubleValidator.StandardNotation
                }
            }
        }
        
        onAccepted: {
            if (selectedAccount && newLimitField.text.length > 0) {
                var newLimit = parseFloat(newLimitField.text)
                if (!isNaN(newLimit) && newLimit >= 0) {
                    if (controller.accountViewModel.setWithdrawLimit(selectedAccount.cardNumber, newLimit)) {
                        accountListView.model = controller.accountViewModel.getAllAccounts()
                        // 重新选择相同的账户
                        var newIndex = findAccountIndex(selectedAccount.cardNumber)
                        if (newIndex >= 0) {
                            accountListView.currentIndex = newIndex
                        }
                    }
                }
            }
        }
    }
    
    // 删除账户确认对话框
    Dialog {
        id: deleteDialog
        title: "确认删除账户"
        standardButtons: Dialog.Yes | Dialog.No
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width - 50, 400)
        
        Label {
            text: "确定要删除 " + (selectedAccount ? selectedAccount.holderName : "") + " 的账户吗？\n此操作不可撤销！"
            wrapMode: Text.WordWrap
            width: parent.width
            color: "#f44336"
        }
        
        onAccepted: {
            if (selectedAccount) {
                if (controller.accountViewModel.deleteAccount(selectedAccount.cardNumber)) {
                    // 更新列表并选择第一个账户
                    accountListView.model = controller.accountViewModel.getAllAccounts()
                    if (accountListView.model.length > 0) {
                        accountListView.currentIndex = 0
                    } else {
                        selectedAccount = null
                    }
                }
            }
        }
    }
    
    // 创建新账户对话框
    Dialog {
        id: newAccountDialog
        title: "创建新账户"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width - 50, 400)
        
        ColumnLayout {
            spacing: 10
            width: parent.width
            
            TextField {
                id: cardNumberField
                placeholderText: "卡号 (16位数字)"
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 16
                
                validator: RegularExpressionValidator {
                    regularExpression: /^[0-9]{0,16}$/
                }
            }
            
            TextField {
                id: pinField
                placeholderText: "PIN码 (4位数字)"
                Layout.fillWidth: true
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 4
                
                validator: RegularExpressionValidator {
                    regularExpression: /^[0-9]{0,4}$/
                }
            }
            
            TextField {
                id: holderNameField
                placeholderText: "持卡人姓名"
                Layout.fillWidth: true
            }
            
            TextField {
                id: balanceField
                placeholderText: "初始余额"
                Layout.fillWidth: true
                text: "0.00"
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                
                validator: DoubleValidator {
                    bottom: 0
                    notation: DoubleValidator.StandardNotation
                }
            }
            
            TextField {
                id: withdrawLimitField
                placeholderText: "取款限额"
                Layout.fillWidth: true
                text: "2000.00"
                inputMethodHints: Qt.ImhFormattedNumbersOnly
                
                validator: DoubleValidator {
                    bottom: 0
                    notation: DoubleValidator.StandardNotation
                }
            }
            
            CheckBox {
                id: isLockedCheck
                text: "账户锁定"
                checked: false
            }
            
            CheckBox {
                id: isAdminCheck
                text: "管理员账户"
                checked: false
            }
        }
        
        onAccepted: {
            if (cardNumberField.text.length === 16 && 
                pinField.text.length === 4 && 
                holderNameField.text.length > 0) {
                
                var initialBalance = parseFloat(balanceField.text) || 0
                var withdrawLimit = parseFloat(withdrawLimitField.text) || 2000
                
                if (controller.accountViewModel.createAccount(
                        cardNumberField.text,
                        pinField.text,
                        holderNameField.text,
                        initialBalance,
                        withdrawLimit,
                        isLockedCheck.checked,
                        isAdminCheck.checked)) {
                    
                    // 清空字段
                    cardNumberField.text = ""
                    pinField.text = ""
                    holderNameField.text = ""
                    balanceField.text = "0.00"
                    withdrawLimitField.text = "2000.00"
                    isLockedCheck.checked = false
                    isAdminCheck.checked = false
                    
                    // 更新列表
                    accountListView.model = controller.accountViewModel.getAllAccounts()
                }
            }
        }
        
        onRejected: {
            // 清空字段
            cardNumberField.text = ""
            pinField.text = ""
            holderNameField.text = ""
            balanceField.text = "0.00"
            withdrawLimitField.text = "2000.00"
            isLockedCheck.checked = false
            isAdminCheck.checked = false
        }
    }
} 