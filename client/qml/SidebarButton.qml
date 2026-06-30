import QtQuick
import QtQuick.Controls

Button {
    id: root

    property string title: ""
    property string iconText: ""
    property bool isActive: false

    property string activeColor: "#5B6EF5"
    property string activeColorEnd: "#7C8CFF"
    property string hoveredColor: "#EEF0FE"
    property color inactiveColor: Qt.rgba(1, 1, 1, 0)
    property string pressedColor: "#E3E6FC"
    property string textColor: "#5B6EF5"

    signal itemClicked(string title)

    height: 48
    width: parent.width

    background: Rectangle {
        id: bg
        anchors.fill: parent
        radius: 12
        color: root.isActive ? "transparent" : (root.pressed ? root.pressedColor : (root.hovered ? root.hoveredColor : root.inactiveColor))

        Behavior on color { ColorAnimation { duration: 150 } }

        gradient: root.isActive ? activeGradient : null

        Gradient {
            id: activeGradient
            orientation: Gradient.Horizontal
            GradientStop { position: 0.0; color: root.activeColor }
            GradientStop { position: 1.0; color: root.activeColorEnd }
        }

        Rectangle {
            visible: root.isActive
            anchors.fill: parent
            anchors.topMargin: 4
            radius: parent.radius
            color: root.activeColor
            opacity: 0.25
            z: -1
            scale: 1.03
        }

        Rectangle {
            visible: root.isActive
            width: 3
            height: parent.height * 0.5
            radius: 2
            color: "#FFFFFF"
            opacity: 0.9
            anchors.left: parent.left
            anchors.leftMargin: 6
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    contentItem: Item {
        Rectangle {
            id: iconBubble
            visible: root.iconText !== ""
            width: 28
            height: 28
            radius: 9
            anchors.left: parent.left
            anchors.leftMargin: 12
            anchors.verticalCenter: parent.verticalCenter
            color: root.isActive ? Qt.rgba(1, 1, 1, 0.18) : "transparent"

            Text {
                anchors.centerIn: parent
                text: root.iconText
                font.pixelSize: 16
                color: root.isActive ? "#FFFFFF" : root.textColor
            }
        }

        Text {
            text: root.title
            color: root.isActive ? "#FFFFFF" : root.textColor
            font.pixelSize: 14
            font.weight: root.isActive ? Font.DemiBold : Font.Medium
            anchors.left: root.iconText !== "" ? iconBubble.right : parent.left
            anchors.leftMargin: root.iconText !== "" ? 10 : 18
            anchors.right: arrowItem.left
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            elide: Text.ElideRight

            Behavior on color { ColorAnimation { duration: 150 } }
        }

        Text {
            id: arrowItem
            text: "›"
            font.pixelSize: 20
            font.bold: true
            color: "#FFFFFF"
            opacity: root.isActive ? 1.0 : 0.0
            anchors.right: parent.right
            anchors.rightMargin: 14
            anchors.verticalCenter: parent.verticalCenter

            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    onClicked: root.itemClicked(root.title)

    scale: root.pressed ? 0.97 : 1.0
    Behavior on scale { NumberAnimation { duration: 100; easing.type: Easing.OutQuad } }
}