import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

RowLayout {
    spacing: 10
    Layout.alignment: Qt.AlignHCenter
    
    property alias confirmText: confirmButton.text
    property alias cancelText: cancelButton.text
    property bool isProcessing: false
    
    signal confirmed()
    signal canceled()
    
    Button {
        id: confirmButton
        text: "确认"
        Layout.preferredWidth: 130
        Layout.preferredHeight: 50
        font.pixelSize: 16
        enabled: !isProcessing
        
        background: Rectangle {
            color: parent.enabled ? "#4CAF50" : "#9E9E9E"
            radius: 5
        }
        
        onClicked: confirmed()
    }
    
    Button {
        id: cancelButton
        text: "取消"
        Layout.preferredWidth: 130
        Layout.preferredHeight: 50
        font.pixelSize: 16
        enabled: !isProcessing
        
        background: Rectangle {
            color: "#2196F3"
            radius: 5
        }
        
        onClicked: canceled()
    }
} 