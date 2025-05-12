import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: keypadColumn
    spacing: 8
    
    property alias displayText: display.text
    property bool clearOnNextKey: false
    property bool isAmount: false
    property real maxValue: 999999.0
    
    signal accepted()
    signal canceled()
    
    // 显示区域
    TextField {
        id: display
        Layout.fillWidth: true
        readOnly: true
        horizontalAlignment: TextInput.AlignRight
        font.pixelSize: 22
        echoMode: isAmount ? TextInput.Normal : TextInput.Password
        Layout.preferredHeight: 50
        placeholderText: isAmount ? "请输入金额" : "请输入数字"
        background: Rectangle {
            color: "#2a2d3a"
            radius: 5
            border.color: "#3a3d4a"
            border.width: 1
        }
        color: "white"
        
        // Prevent focus to avoid IME keyboard popup
        activeFocusOnTab: false
        focus: false
        
        // 增加显示宽度以支持更多位数
        Layout.preferredWidth: 280 // 增加宽度以支持更长的金额数字
    }
    
    // 直接定义完整的布局，不使用Grid
    ColumnLayout {
        id: keypad
        spacing: 8
        Layout.alignment: Qt.AlignHCenter
        
        // 第一行：7, 8, 9
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                text: "7"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("7")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "8"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("8")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "9"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("9")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
        }
        
        // 第二行：4, 5, 6
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                text: "4"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("4")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "5"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("5")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "6"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("6")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
        }
        
        // 第三行：1, 2, 3
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                text: "1"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("1")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "2"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("2")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "3"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("3")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
        }
        
        // 第四行：0, ., 清除
        RowLayout {
            spacing: 8
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                text: "0"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: appendNumber("0")
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "."
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                enabled: isAmount
                visible: isAmount
                onClicked: {
                    if (isAmount && !display.text.includes(".")) {
                        if (display.text === "") {
                            display.text = "0."
                        } else {
                            display.text = display.text + "."
                        }
                    }
                }
                background: Rectangle {
                    color: "#2a2d3a"
                    radius: 5
                    border.color: "#3a3d4a"
                    border.width: 1
                }
            }
            
            Button {
                text: "清除"
                font.pixelSize: 18
                implicitWidth: 75
                implicitHeight: 50
                onClicked: {
                    display.text = ""
                }
                background: Rectangle {
                    color: "#E53935"  // Material Red 600
                    radius: 5
                    border.color: "#C62828"  // Material Red 800
                    border.width: 1
                }
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }
    
    function appendNumber(number) {
        if (clearOnNextKey) {
            display.text = ""
            clearOnNextKey = false
        }
        
        // For amount input, check if we exceed maximum
        if (isAmount) {
            // Handle special case for first digit after decimal point
            if (display.text.endsWith(".") && display.text.length > 0) {
                display.text = display.text + number
                console.log("添加小数点后的数字: " + display.text)
                return
            }
            
            // Check if there are already 2 digits after decimal point
            const decimalIndex = display.text.indexOf(".")
            if (decimalIndex !== -1 && (display.text.length - decimalIndex > 2)) {
                console.log("小数点后已有两位，不再添加: " + display.text)
                return
            }
            
            // 取消金额上限的检查，允许输入任意金额
            let newText = display.text + number
            
            // 增加调试输出
            console.log("尝试添加数字: " + number + " 到 " + display.text + " 结果=" + newText)
            
            // Don't allow more than 2 decimal places
            if (decimalIndex !== -1 && (newText.length - decimalIndex > 3)) {
                console.log("小数位数超过限制")
                return
            }
            
            // 直接设置文本，不再检查最大值
            display.text = newText
            console.log("数字已添加，当前显示: " + display.text)
        } else {
            display.text = display.text + number
        }
    }
    
    function clear() {
        display.text = ""
    }
} 