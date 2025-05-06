import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15

Button {
    id: root
    
    property string iconName: ""
    property string buttonText: ""
    
    Layout.preferredWidth: 220
    Layout.preferredHeight: 80
    
    contentItem: RowLayout {
        spacing: 10
        
        Label {
            text: root.buttonText
            font.pixelSize: 16
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
    
    background: Rectangle {
        color: root.down ? Material.color(Material.Blue, Material.Shade400) :
                          (root.hovered ? Material.color(Material.Blue, Material.Shade300) : 
                                         Material.color(Material.Blue, Material.Shade200))
        radius: 5
        
        // Simple animation for button press
        Behavior on color {
            ColorAnimation { duration: 100 }
        }
    }
} 