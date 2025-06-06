import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import "components"

Page {
    id: loginPage
    title: qsTr("请登录")
    
    // Enable key handling for the entire page
    focus: true
    Keys.onReturnPressed: loginButton.clicked()
    
    background: Rectangle {
        color: "#1e2029"
    }
    
    // Header
    Rectangle {
        id: header
        width: parent.width
        height: 80  // 减小头部高度
        color: Material.color(Material.Blue, Material.Shade700)
        
        Label {
            anchors.centerIn: parent
            text: "ATM 模拟器 - 请刷卡或输入卡号"
            font.pixelSize: 22  // 减小字体大小
            color: "white"
        }
    }
    
    // 使用ScrollView包装内容，确保在小屏幕上可以滚动
    ScrollView {
        id: scrollView
        anchors.fill: parent
        anchors.topMargin: header.height
        contentWidth: parent.width
        contentHeight: contentLayout.height
        clip: true
        
        ColumnLayout {
            id: contentLayout
            width: parent.width
            spacing: 15  // 减少间距
            
            Item { 
                Layout.fillWidth: true
                height: 20  // 减少顶部空间
            }
            
            Label {
                text: "请输入卡号"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
            }
            
            TextField {
                id: cardNumberField
                placeholderText: "16位银行卡号"
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 16
                Layout.preferredWidth: 300
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: TextInput.AlignHCenter
                font.pixelSize: 18
                
                onTextChanged: {
                    // 添加 null 检查
                    if (controller && controller.accountViewModel) {
                        controller.accountViewModel.clearError()
                    }
                }
                
                onAccepted: {
                    pinField.focus = true
                }
            }
            
            Label {
                text: "请输入PIN码"
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
            }
            
            TextField {
                id: pinField
                placeholderText: "4位PIN码"
                echoMode: TextInput.Password
                inputMethodHints: Qt.ImhDigitsOnly
                maximumLength: 4
                Layout.preferredWidth: 200
                Layout.alignment: Qt.AlignHCenter
                horizontalAlignment: TextInput.AlignHCenter
                font.pixelSize: 18
                
                onTextChanged: {
                    // 添加 null 检查
                    if (controller && controller.accountViewModel) {
                        controller.accountViewModel.clearError()
                    }
                }
                
                onAccepted: {
                    loginButton.clicked()
                }
            }
            
            Button {
                id: loginButton
                text: "登录"
                Layout.preferredWidth: 200
                Layout.preferredHeight: 50
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 16
                
                onClicked: {
                    // 添加 null 检查
                    if (!controller || !controller.accountViewModel) {
                        console.error("controller 或 accountViewModel 为 null")
                        return
                    }
                    
                    controller.accountViewModel.clearError()
                    
                    if (cardNumberField.text.length === 0) {
                        controller.accountViewModel.setErrorMessage("请输入卡号")
                        cardNumberField.focus = true
                        return
                    }
                    
                    if (pinField.text.length === 0) {
                        controller.accountViewModel.setErrorMessage("请输入PIN码")
                        pinField.focus = true
                        return
                    }
                    
                    // 检查是否为管理员卡号
                    if (cardNumberField.text === "9999888877776666") {
                        // 使用管理员登录方法
                        if (controller.accountViewModel.adminLogin(cardNumberField.text, pinField.text)) {
                            pinField.text = ""
                            controller.switchToPage("AdminPage")
                        }
                    } else {
                        // 普通账户登录
                        if (controller.accountViewModel.loginWithCard(cardNumberField.text, pinField.text)) {
                            pinField.text = ""
                            // 检查是否是管理员，如果是则直接跳转到AdminPage
                            if (controller.accountViewModel.isAdmin) {
                                controller.switchToPage("AdminPage")
                            } else {
                                controller.switchToPage("MainMenu")
                            }
                        }
                    }
                }
            }
            
            // Show error message if any
            Label {
                id: errorLabel
                // 添加 null 检查
                text: controller && controller.accountViewModel ? controller.accountViewModel.errorMessage : ""
                color: "red"
                visible: text !== ""
                Layout.alignment: Qt.AlignHCenter
                font.pixelSize: 16
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                wrapMode: Text.WordWrap
            }
            
            Item { 
                Layout.fillWidth: true
                height: 10  // 减少间距
            }
            
            // Test account information link
            Label {
                id: accountInfoLink
                text: "<a href='#'>显示测试账户信息</a>"
                Layout.alignment: Qt.AlignHCenter
                Layout.topMargin: 5
                font.pixelSize: 14
                color: "#4b8bf4"
                
                onLinkActivated: {
                    testAccountInfo.visible = !testAccountInfo.visible
                    text = testAccountInfo.visible ? "<a href='#'>隐藏测试账户信息</a>" : "<a href='#'>显示测试账户信息</a>"
                }
            }
            
            // Test account information (for demo purposes) - initially hidden
            Rectangle {
                id: testAccountInfo
                visible: false
                color: "#2a2d3a"
                radius: 5
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)  // 响应式宽度
                Layout.preferredHeight: 125 // 增加高度以适应新的管理员账户
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    
                    Label {
                        text: "测试账户信息："
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                    }
                    
                    Label {
                        text: "卡号: 1234567890123456   PIN: 1234   (正常账户)"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        font.pixelSize: 14  // 减小字体
                    }
                    
                    Label {
                        text: "卡号: 2345678901234567   PIN: 2345   (余额较高)"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        font.pixelSize: 14  // 减小字体
                    }
                    
                    Label {
                        text: "卡号: 3456789012345678   PIN: 3456   (已锁定账户)"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        font.pixelSize: 14  // 减小字体
                    }
                    
                    Label {
                        text: "卡号: 9999888877776666   PIN: 8888   (管理员账户)"
                        Layout.alignment: Qt.AlignHCenter
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                        font.pixelSize: 14
                        color: "#f44336" // 红色突出显示管理员账户
                        font.bold: true
                    }
                }
            }
            
            Item { 
                Layout.fillWidth: true
                height: 20  // 底部空间
            }
        }
    }
    
    // Set initial focus to the card number field
    Component.onCompleted: {
        cardNumberField.forceActiveFocus()
    }
    
    // 添加连接器来安全地处理 controller.accountViewModel 的访问
    Connections {
        target: controller ? controller.accountViewModel : null
        function onErrorMessageChanged() {
            // 处理错误消息变化，这样可以避免直接绑定到 controller.accountViewModel.errorMessage
        }
    }
}