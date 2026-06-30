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
        color: "#EDEFF7"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // шапка и логотип
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 76

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 10

                Rectangle {
                    width: 36
                    height: 36
                    radius: 11
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: "#5B6EF5" }
                        GradientStop { position: 1.0; color: "#7C8CFF" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✦"
                        color: "#FFFFFF"
                        font.pixelSize: 16
                        font.bold: true
                    }
                }

                Text {
                    text: "Мессенджер"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    color: "#1A1D29"
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#F2F3FA"
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 16
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: menuColumn.height

            Column {
                id: menuColumn
                width: parent.width
                spacing: 6

                SidebarButton {
                    width: parent.width - 24
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Общий чат"
                    iconText: "💬"
                    isActive: root.currentIndex === 0
                    visible: true

                    textColor: "#5B6EF5"

                    onItemClicked: {
                        root.currentIndex = 0
                    }
                }

                SidebarButton {
                    width: parent.width - 24
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Друзья"
                    iconText: "👥"
                    isActive: root.currentIndex === 1
                    visible: true

                    textColor: "#5B6EF5"

                    onItemClicked: {
                        root.currentIndex = 1
                    }
                }

                SidebarButton {
                    width: parent.width - 24
                    anchors.horizontalCenter: parent.horizontalCenter
                    title: "Чаты"
                    iconText: "🗂"
                    isActive: root.currentIndex === 2
                    visible: true

                    textColor: "#5B6EF5"

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

        Rectangle {
            Layout.fillWidth: true
            Layout.leftMargin: 20
            Layout.rightMargin: 20
            height: 1
            color: "#F2F3FA"
        }

        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: logoutButton.height + 20
            Layout.bottomMargin: 0

            SidebarButton {
                id: logoutButton
                width: parent.width - 24
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20
                title: "Выйти"
                iconText: "🚪"
                isActive: false

                activeColor: "#FF5C72"
                activeColorEnd: "#FF7A8C"
                hoveredColor: "#FFF0F2"
                inactiveColor: Qt.rgba(1, 1, 1, 0)
                pressedColor: "#FFE3E7"
                textColor: "#FF5C72"

                onItemClicked: {
                    ClientHandler.logout()
                    stackView.replace("AuthScreen.qml")
                }
            }
        }
    }
}