import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

ColumnLayout {
    id: keypadColumn
    spacing: 8
    
    property alias displayText: display.text
    property bool clearOnNextKey: false
    property bool isAmount: false
    property real maxValue: 10000.0
    
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
    }
    
    // 数字键盘网格
    Grid {
        id: keypad
        columns: 3
        spacing: 8
        Layout.alignment: Qt.AlignHCenter
        
        // 按照标准ATM键盘布局 (789/456/123/0.C)
        // 创建7-9按钮
        Repeater {
            model: ["7", "8", "9", "4", "5", "6", "1", "2", "3"]
            
            Button {
                text: modelData
                font.pixelSize: 18
                width: 75
                height: 50
                
                onClicked: appendNumber(text)
            }
        }
        
        // 零按钮
        Button {
            text: "0"
            font.pixelSize: 18
            width: 75
            height: 50
            
            onClicked: appendNumber(text)
        }
        
        // 小数点按钮
        Button {
            text: "."
            font.pixelSize: 18
            width: 75
            height: 50
            enabled: isAmount
            visible: isAmount
            
            onClicked: {
                if (isAmount && !display.text.includes(".")) {
                    display.text = display.text + "."
                }
            }
        }
        
        // 清除按钮
        Button {
            text: "清除"
            font.pixelSize: 18
            width: 75
            height: 50
            
            onClicked: {
                display.text = ""
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
            let newValue = parseFloat(display.text + number)
            if (isNaN(newValue) || newValue <= maxValue) {
                display.text = display.text + number
            }
        } else {
            display.text = display.text + number
        }
    }
    
    function clear() {
        display.text = ""
    }
} 