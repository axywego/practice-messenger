import QtQuick
import QtQuick.Controls

Button {
    id: root
    
    property string title: ""
    property string iconText: ""
    property bool isActive: false
    
    property string activeColor: "#1976D2"
    property string hoveredColor: "#1565C0"
    property string inactiveColor: "#E8F0FE"
    property string pressedColor: "#0D47A1"
    property string textColor: "#1976D2"
    
    signal itemClicked(string title)

    height: 48
    width: parent.width
    
    background: Rectangle {
        anchors.fill: parent
        radius: 10
        color: {
            if (root.isActive) return root.activeColor
            if (root.pressed) return root.pressedColor
            if (root.hovered) return root.hoveredColor
            return root.inactiveColor
        }
        
        border.width: 0
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }

    contentItem: Item {
        Text {
            id: iconTextItem
            text: root.iconText
            font.pixelSize: 20
            color: root.isActive ? "#FFFFFF" : root.textColor
            visible: root.iconText !== ""
            anchors.left: parent.left
            anchors.leftMargin: 16
            anchors.verticalCenter: parent.verticalCenter
        }
        
        Text {
            text: root.title
            color: root.isActive ? "#FFFFFF" : root.textColor
            font.pixelSize: 14
            font.weight: root.isActive ? Font.DemiBold : Font.Normal
            anchors.left: root.iconText !== "" ? iconTextItem.right : parent.left
            anchors.leftMargin: root.iconText !== "" ? 12 : 16
            anchors.right: arrowItem.left
            anchors.rightMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight
        }
        
        Text {
            id: arrowItem
            text: "›"
            font.pixelSize: 24
            color: "#FFFFFF"
            opacity: root.isActive ? 1.0 : 0.0
            visible: root.isActive
            anchors.right: parent.right
            anchors.rightMargin: 16
            anchors.verticalCenter: parent.verticalCenter
            
            Behavior on opacity {
                NumberAnimation { duration: 150 }
            }
        }
    }

    onClicked: {
        root.itemClicked(root.title)
    }
    
    scale: root.pressed ? 0.98 : 1.0
    
    Behavior on scale {
        NumberAnimation { duration: 100 }
    }
}