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
        title: "修改密码"
        showBackButton: true
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        
        onBackClicked: controller.switchToPage("MainMenu")
    }
    
    // 使用ScrollView确保在小屏幕上内容可滚动
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
                height: 20
            }
            
            // Change Password Form
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)  // 响应式宽度
                Layout.preferredHeight: 400  // 调整高度
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                color: Material.color(Material.BlueGrey, Material.Shade800)
                
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15
                    
                    Label {
                        text: "修改PIN码"
                        font.pixelSize: 24
                        font.bold: true
                        Layout.alignment: Qt.AlignHCenter
                        color: "white"
                    }
                    
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Material.color(Material.Grey, Material.Shade400)
                    }
                    
                    Label {
                        text: "当前PIN码"
                        font.pixelSize: 16
                        color: "white"
                    }
                    
                    TextField {
                        id: currentPinField
                        placeholderText: "输入当前PIN码"
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhDigitsOnly
                        maximumLength: 4
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "#2a2d3a"
                            radius: 5
                            border.color: "#3a3d4a"
                            border.width: 1
                        }
                        color: "white"
                        
                        onTextChanged: {
                            controller.accountViewModel.clearError()
                        }
                        
                        onAccepted: {
                            newPinField.focus = true
                        }
                    }
                    
                    Label {
                        text: "新PIN码"
                        font.pixelSize: 16
                        color: "white"
                    }
                    
                    TextField {
                        id: newPinField
                        placeholderText: "输入新的4位PIN码"
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhDigitsOnly
                        maximumLength: 4
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "#2a2d3a"
                            radius: 5
                            border.color: "#3a3d4a"
                            border.width: 1
                        }
                        color: "white"
                        
                        onTextChanged: {
                            controller.accountViewModel.clearError()
                        }
                        
                        onAccepted: {
                            confirmPinField.focus = true
                        }
                    }
                    
                    Label {
                        text: "确认新PIN码"
                        font.pixelSize: 16
                        color: "white"
                    }
                    
                    TextField {
                        id: confirmPinField
                        placeholderText: "再次输入新PIN码"
                        echoMode: TextInput.Password
                        inputMethodHints: Qt.ImhDigitsOnly
                        maximumLength: 4
                        Layout.fillWidth: true
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "#2a2d3a"
                            radius: 5
                            border.color: "#3a3d4a"
                            border.width: 1
                        }
                        color: "white"
                        
                        onTextChanged: {
                            controller.accountViewModel.clearError()
                        }
                        
                        onAccepted: {
                            changePasswordButton.clicked()
                        }
                    }
                    
                    // Error message
                    Label {
                        id: errorLabel
                        text: controller.accountViewModel.errorMessage
                        color: "red"
                        visible: text !== ""
                        Layout.alignment: Qt.AlignHCenter
                        font.pixelSize: 16
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                    }
                    
                    // 按钮使用RowLayout以保持一致的布局
                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        spacing: 20
                        
                        Item { Layout.fillWidth: true }
                        
                        Button {
                            id: changePasswordButton
                            text: "确认修改"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 50
                            font.pixelSize: 16
                            
                            background: Rectangle {
                                color: "#4CAF50"  // Green
                                radius: 5
                            }
                            
                            onClicked: {
                                if (controller.accountViewModel.changePassword(
                                        currentPinField.text, 
                                        newPinField.text,
                                        confirmPinField.text)) {
                                    // Success, show dialog
                                    successDialog.open()
                                }
                            }
                        }
                        
                        Button {
                            text: "取消"
                            Layout.preferredWidth: 160
                            Layout.preferredHeight: 50
                            font.pixelSize: 16
                            
                            background: Rectangle {
                                color: "#2196F3"  // Blue
                                radius: 5
                            }
                            
                            onClicked: {
                                controller.switchToPage("MainMenu")
                            }
                        }
                        
                        Item { Layout.fillWidth: true }
                    }
                }
            }
            
            // 底部边距
            Item { 
                Layout.fillWidth: true 
                height: 20
            }
        }
    }
    
    // Success dialog
    Dialog {
        id: successDialog
        title: "操作成功"
        modal: true
        anchors.centerIn: parent
        width: 300
        
        contentItem: ColumnLayout {
            spacing: 20
            
            Label {
                text: "PIN码修改成功！"
                font.pixelSize: 16
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
        }
        
        footer: DialogButtonBox {
            Button {
                text: "确定"
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
        }
        
        onAccepted: {
            // Clear fields and return to main menu
            currentPinField.text = ""
            newPinField.text = ""
            confirmPinField.text = ""
            controller.switchToPage("MainMenu")
        }
    }
} 