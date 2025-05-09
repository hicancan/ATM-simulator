import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import QtQuick.Window 2.15

ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 650  // 减小高度以适应Windows任务栏
    minimumWidth: 800
    minimumHeight: 600  // 进一步减小最小高度以适应更多屏幕
    title: qsTr("ATM 模拟器")
    flags: Qt.Window | Qt.WindowTitleHint | Qt.WindowSystemMenuHint | 
          Qt.WindowMinimizeButtonHint | Qt.WindowMaximizeButtonHint | 
          Qt.WindowCloseButtonHint | Qt.WindowFullscreenButtonHint
    
    // 设置窗口位置居中，避免被任务栏遮挡
    Component.onCompleted: {
        // 获取屏幕尺寸
        let screenWidth = Screen.width
        let screenHeight = Screen.height
        
        // 计算居中位置，考虑Windows任务栏高度（通常约40像素）
        let taskbarHeight = 40
        let x = (screenWidth - width) / 2
        let y = (screenHeight - height - taskbarHeight) / 2
        
        // 设置窗口位置
        setX(x)
        setY(y)
        
        // 如果屏幕分辨率较低，调整窗口大小
        if (screenHeight < 800) {
            height = screenHeight - taskbarHeight - 50 // 留出一些边距
        }
    }
    
    // Set material theme
    Material.theme: Material.Dark
    Material.accent: Material.Blue
    
    // Connect to the page change signal
    Connections {
        target: controller
        function onCurrentPageChanged() {
            stackView.replace(controller.currentPage + ".qml")
        }
    }
    
    // Header with application information
    header: ToolBar {
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            
            Label {
                text: "ATM 模拟器"
                font.pixelSize: 20
                elide: Label.ElideRight
                horizontalAlignment: Qt.AlignLeft
                verticalAlignment: Qt.AlignVCenter
                Layout.fillWidth: true
            }
            
            Label {
                id: timeLabel
                text: new Date().toLocaleTimeString(Qt.locale(), "hh:mm:ss")
                horizontalAlignment: Qt.AlignRight
                Layout.alignment: Qt.AlignRight
                font.pixelSize: 16
                
                Timer {
                    interval: 1000
                    running: true
                    repeat: true
                    onTriggered: timeLabel.text = new Date().toLocaleTimeString(Qt.locale(), "hh:mm:ss")
                }
            }
            
            ToolButton {
                text: qsTr("退出")
                // 添加 null 检查
                visible: controller && controller.accountViewModel && controller.accountViewModel.isLoggedIn
                onClicked: logoutDialog.open()
            }
            
            Item {
                width: 10
            }
        }
    }
    
    // Main content area with stack for page switching
    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: "LoginPage.qml"
    }
    
    // Dialog for confirming logout
    Dialog {
        id: logoutDialog
        title: "确认退出"
        standardButtons: Dialog.Yes | Dialog.No
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        
        Label {
            text: "是否确认退出系统？请保存好您的银行卡！"
        }
        
        onAccepted: controller.logout()
    }
    
    // Handle back button navigation
    onClosing: function(close) {
        // 添加 null 检查
        if (controller && controller.accountViewModel && controller.accountViewModel.isLoggedIn) {
            logoutDialog.open()
            close.accepted = false
        }
    }
}