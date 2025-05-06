import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Rectangle {
    id: headerBar
    width: parent.width
    height: 60
    color: Material.color(Material.Blue, Material.Shade800)
    
    property string title: ""
    property bool showBackButton: false
    
    signal backClicked()
    
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        
        Button {
            text: "返回"
            visible: showBackButton
            onClicked: backClicked()
            
            background: Rectangle {
                color: parent.down ? Material.color(Material.Blue, Material.Shade600) :
                                    Material.color(Material.Blue, Material.Shade800)
                radius: 4
            }
        }
        
        Label {
            text: title
            font.pixelSize: 22
            color: "white"
            Layout.fillWidth: true
            horizontalAlignment: Qt.AlignHCenter
        }
        
        // Spacer for symmetry with back button
        Item {
            width: showBackButton ? 50 : 0
            visible: showBackButton
        }
    }
} 