import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    
    color: "#FFFFFF"
    width: 260
    
    property int currentIndex: 0
    
    Rectangle {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: 1
        color: "#E0E6ED"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 20
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: menuColumn.height
            
            Column {
                id: menuColumn
                width: parent.width
                spacing: 6
                
                SidebarButton {
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Общий чат"
                    isActive: root.currentIndex === 0
                    visible: true

                    textColor: root.currentIndex === 0 ? "#FFFFFF" : "#1976D2"

                    onItemClicked: {
                        root.currentIndex = 0
                    }
                }

                SidebarButton {
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Друзья"
                    isActive: root.currentIndex === 1
                    visible: true
                    
                    textColor: root.currentIndex === 1 ? "#FFFFFF" : "#1976D2"

                    onItemClicked: {
                        root.currentIndex = 1
                    }
                }

                SidebarButton {
                    width: parent.width - 20
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Чаты"
                    isActive: root.currentIndex === 2
                    visible: true
                    
                    textColor: root.currentIndex === 2 ? "#FFFFFF" : "#1976D2"

                    onItemClicked: {
                        root.currentIndex = 2
                    }
                }
            }
        }
        
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: logoutButton.height + 20
            Layout.bottomMargin: 0
            
            SidebarButton {
                id: logoutButton
                width: parent.width - 20
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                title: "Выйти"
                iconText: "🚪"
                isActive: false
                
                activeColor: "#F44336"
                hoveredColor: "#E53935"
                inactiveColor: "#FFEBEE"
                pressedColor: "#B71C1C"
                textColor: "#F44336"

                onItemClicked: {
                    ClientHandler.logout()
                    stackView.replace("AuthScreen.qml")
                }
            }
        }
    }
}