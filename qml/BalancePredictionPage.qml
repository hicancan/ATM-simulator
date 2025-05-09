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
            
            // 预测方法说明面板
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                color: Material.color(Material.BlueGrey, Material.Shade800)
                implicitHeight: methodInfoColumn.implicitHeight + 40

                ColumnLayout {
                    id: methodInfoColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 10

                    Label {
                        text: "预测方法说明"
                        font.pixelSize: 16
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
                        text: "当前使用加权平均法和线性回归相结合的方式预测余额。近期交易会获得更高权重，提高预测准确度。"
                        font.pixelSize: 14
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                        color: "white"
                    }
                }
            }
            
            // 多日期余额预测面板
            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredWidth: Math.min(parent.width - 40, 500)
                Layout.leftMargin: 20
                Layout.rightMargin: 20
                radius: 10
                color: Material.color(Material.Teal, Material.Shade800)
                implicitHeight: multiPredictionColumn.implicitHeight + 40

                ColumnLayout {
                    id: multiPredictionColumn
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    Label {
                        text: "多日期余额预测"
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
                    
                    // 多日期预测选择
                    ComboBox {
                        id: predictionDaysCombo
                        Layout.fillWidth: true
                        model: ["7,14,30,90", "1,3,7,14", "30,60,90,180", "7,30,90,365"]
                        currentIndex: 0
                        
                        delegate: ItemDelegate {
                            width: predictionDaysCombo.width
                            contentItem: Text {
                                text: modelData
                                color: "black"
                                font.pixelSize: 14
                                elide: Text.ElideRight
                                verticalAlignment: Text.AlignVCenter
                            }
                            highlighted: predictionDaysCombo.highlightedIndex === index
                        }
                        
                        contentItem: Text {
                            leftPadding: 10
                            rightPadding: predictionDaysCombo.indicator.width + predictionDaysCombo.spacing
                            text: predictionDaysCombo.displayText
                            font.pixelSize: 14
                            color: "black"
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                        }
                    }
                    
                    // 自定义日期输入字段
                    TextField {
                        id: customDaysField
                        Layout.fillWidth: true
                        placeholderText: "自定义预测天数 (例如: 7,14,30,90)"
                        font.pixelSize: 14
                        color: "black"
                        Material.accent: Material.Teal
                    }
                    
                    // 计算按钮
                    Button {
                        text: "计算预测"
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: 180
                        Layout.preferredHeight: 40
                        font.pixelSize: 14
                        Material.background: Material.Orange
                        enabled: controller.accountViewModel.isLoggedIn
                        
                        onClicked: {
                            // 使用自定义输入或下拉框选择的值
                            let daysToPredict = customDaysField.text.trim() !== "" ? 
                                customDaysField.text : predictionDaysCombo.currentText;
                            controller.accountViewModel.calculateMultiDayPredictions(daysToPredict); 
                        }
                    }
                    
                    // 日期预测结果网格
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 2
                        columnSpacing: 10
                        rowSpacing: 10
                        visible: Object.keys(controller.accountViewModel.multiDayPredictions).length > 0
                        
                        // 动态创建预测结果项
                        Repeater {
                            model: {
                                // 从multiDayPredictions中提取键值对并按天数排序
                                let predictions = controller.accountViewModel.multiDayPredictions;
                                let sortedKeys = Object.keys(predictions).sort((a, b) => parseInt(a) - parseInt(b));
                                let result = [];
                                
                                for (let i = 0; i < sortedKeys.length; i++) {
                                    let day = sortedKeys[i];
                                    result.push({
                                        day: day,
                                        amount: predictions[day]
                                    });
                                }
                                
                                return result;
                            }
                            
                            delegate: RowLayout {
                                Layout.fillWidth: true
                                Layout.columnSpan: 2
                                
                                Label {
                                    text: modelData.day + " 天后:"
                                    font.pixelSize: 14
                                    color: "white"
                                    Layout.preferredWidth: 80
                                }
                                
                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 30
                                    color: Material.color(Material.Teal, Material.Shade700)
                                    radius: 4
                                    
                                    Label {
                                        anchors.fill: parent
                                        anchors.leftMargin: 10
                                        anchors.rightMargin: 10
                                        text: "￥" + modelData.amount.toFixed(2)
                                        font.pixelSize: 14
                                        font.bold: true
                                        color: modelData.amount > controller.accountViewModel.balance ? 
                                               "#4caf50" : (modelData.amount < controller.accountViewModel.balance ? 
                                               "#f44336" : "white")
                                        horizontalAlignment: Text.AlignRight
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                }
                            }
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