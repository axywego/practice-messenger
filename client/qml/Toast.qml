import QtQuick
import QtQuick.Layouts

Rectangle {
    id: item
    
    property string message: ""
    property string toastType: "error"
    property int duration: 4000
    
    signal dismissed()
    
    radius: 14
    color: "#FFFFFF"
    border.width: 1
    border.color: "#EDEFF7"
    
    width: 380
    height: 60
    
    readonly property var palette_: ({
        "error":   { accent: "#FF5C72", bg: "#FFF0F2", icon: "✕" },
        "success": { accent: "#34C77B", bg: "#E7F9EE", icon: "✓" },
        "info":    { accent: "#5B6EF5", bg: "#EEF0FE", icon: "i" },
        "warning": { accent: "#F5A623", bg: "#FFF6E8", icon: "!" }
    })
    readonly property var style: palette_[toastType] ?? palette_["info"]
    
    visible: true
    opacity: 1
    
    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 5
        radius: parent.radius
        color: "#1A1D29"
        opacity: 0.15
        z: -1
    }
    
    RowLayout {
        id: contentRow
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12
        
        Rectangle {
            width: 28
            height: 28
            radius: 9
            color: item.style.bg
            
            Text {
                anchors.centerIn: parent
                text: item.style.icon
                color: item.style.accent
                font.bold: true
                font.pixelSize: 13
            }
        }
        
        Text {
            text: item.message
            color: "#1A1D29"
            font.pixelSize: 13
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        
        Rectangle {
            width: 24
            height: 24
            radius: 8
            color: closeMouse.containsMouse ? "#F4F6FB" : "transparent"
            
            Text {
                anchors.centerIn: parent
                text: "×"
                font.pixelSize: 15
                color: "#A8ACBD"
            }
            
            MouseArea {
                id: closeMouse
                anchors.fill: parent
                hoverEnabled: true
                onClicked: closeToast()
            }
        }
    }
    
    Rectangle {
        id: progress
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        height: 3
        radius: 2
        color: item.style.accent
        opacity: 0.45
        width: parent.width
        
        NumberAnimation {
            id: progressAnim
            target: progress
            property: "width"
            from: parent.width
            to: 0
            duration: item.duration
            running: false
        }
    }
    
    Timer {
        id: hideTimer
        interval: item.duration
        onTriggered: closeToast()
    }
    
    function closeToast() {
        hideTimer.stop()
        progressAnim.stop()
        item.destroy()
    }
    
    Component.onCompleted: {
        console.log("Toast created:", message, toastType)
        hideTimer.start()
        progressAnim.start()
    }
}