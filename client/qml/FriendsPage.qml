import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0
import Client.ErrorHandler 1.0

Rectangle {
    id: root
    objectName: "FriendsPage"
    color: "#F4F6FB"

    property var friends: []
    property var pending: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Text {
            text: "Друзья"
            font.pixelSize: 20
            font.weight: Font.DemiBold
            color: "#1A1D29"
            Layout.fillWidth: true
        }

        // добавление кента
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: addRow.height + 24
            radius: 14
            color: "#FFFFFF"
            border.width: 1
            border.color: "#EDEFF7"

            RowLayout {
                id: addRow
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.margins: 12
                spacing: 10

                TextField {
                    id: targetLoginField
                    Layout.fillWidth: true
                    placeholderText: "Логин пользователя для дружбы"
                    font.pixelSize: 14
                    leftPadding: 12

                    background: Rectangle {
                        radius: 10
                        color: "#F4F6FB"
                        border.width: targetLoginField.activeFocus ? 1.5 : 0
                        border.color: "#5B6EF5"
                    }
                }

                Button {
                    text: "Отправить"
                    Layout.preferredHeight: 38

                    onClicked: {
                        ClientHandler.sendFriendRequest(targetLoginField.text)
                        targetLoginField.text = ""
                    }

                    background: Rectangle {
                        radius: 10
                        gradient: Gradient {
                            orientation: Gradient.Horizontal
                            GradientStop { position: 0.0; color: "#5B6EF5" }
                            GradientStop { position: 1.0; color: "#7C8CFF" }
                        }
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: 14
                        rightPadding: 14
                    }
                }
            }
        }

        // друзья
        Text {
            text: "Мои друзья"
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: "#8A8FA3"
            Layout.fillWidth: true
            Layout.topMargin: 4
        }

        ListView {
            id: friendList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 100
            spacing: 8
            clip: true

            model: root.friends

            delegate: Rectangle {
                width: friendList.width
                height: 52
                radius: 12
                color: "#FFFFFF"
                border.width: 1
                border.color: "#EDEFF7"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 10

                    Rectangle {
                        width: 30
                        height: 30
                        radius: 10
                        color: "#EEF0FE"

                        Text {
                            anchors.centerIn: parent
                            text: modelData.length > 0 ? modelData.charAt(0).toUpperCase() : "?"
                            color: "#5B6EF5"
                            font.bold: true
                            font.pixelSize: 13
                        }
                    }

                    Text {
                        text: modelData
                        font.bold: true
                        font.pixelSize: 14
                        color: "#1A1D29"
                        Layout.fillWidth: true
                    }
                }
            }
        }

        // ждут ответа
        Text {
            text: "Ожидают ответа"
            font.pixelSize: 14
            font.weight: Font.DemiBold
            color: "#8A8FA3"
            Layout.fillWidth: true
            Layout.topMargin: 4
        }

        ListView {
            id: pendingList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 100
            spacing: 8
            clip: true

            model: root.pending

            delegate: Rectangle {
                width: pendingList.width
                height: 52
                radius: 12
                color: "#FFFFFF"
                border.width: 1
                border.color: "#EDEFF7"

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 12
                    spacing: 10

                    Rectangle {
                        width: 30
                        height: 30
                        radius: 10
                        color: "#EEF0FE"

                        Text {
                            anchors.centerIn: parent
                            text: modelData.length > 0 ? modelData.charAt(0).toUpperCase() : "?"
                            color: "#5B6EF5"
                            font.bold: true
                            font.pixelSize: 13
                        }
                    }

                    Text {
                        text: modelData
                        font.bold: true
                        font.pixelSize: 14
                        color: "#1A1D29"
                        Layout.fillWidth: true
                    }

                    Button {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34

                        background: Rectangle {
                            radius: 10
                            color: "#E7F9EE"
                        }

                        contentItem: Text {
                            text: "✓"
                            color: "#34C77B"
                            font.bold: true
                            font.pixelSize: 15
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            ClientHandler.acceptFriendRequest(modelData)
                            ClientHandler.getFriendList()
                        }
                    }

                    Button {
                        Layout.preferredWidth: 34
                        Layout.preferredHeight: 34

                        background: Rectangle {
                            radius: 10
                            color: "#FFF0F2"
                        }

                        contentItem: Text {
                            text: "✕"
                            color: "#FF5C72"
                            font.bold: true
                            font.pixelSize: 15
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            ClientHandler.rejectFriendRequest(modelData)
                            ClientHandler.getFriendList()
                        }
                    }
                }
            }
        }
    }

    Connections {
        target: ClientBridge

        function onChatMessageReceived(senderLogin, message) {
            chatModel.append({ sender: senderLogin, message: message })
        }

        function onFriendNewRequest() {
            ClientHandler.getFriendList()
        }

        function onFriendListReceived(friends, pending) {
            root.friends = friends
            root.pending = pending
        }

        function onFriendRequestResult(errorCode) {
            if(!ClientErrorHandler.isOk(errorCode))
                window.showToast(ClientErrorHandler.textError(errorCode), "error", 4000)
        }
    }

    Component.onCompleted: {
        ClientHandler.tokenReady.connect(onTokenReady)
        if(ClientHandler.isTokenReady)
            ClientHandler.getFriendList()
    }

    function onTokenReady() {
        ClientHandler.getFriendList()
    }
}