import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "FriendsPage"
    color: "#F5F5F5"

    property var friends: []
    property var pending: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        TextField {
            id: targetLoginField
            Layout.fillWidth: true
            placeholderText: "Логин пользователя для дружбы"
        }

        Button {
            text: "отправить запрос дружбы"

            onClicked: {
                ClientHandler.sendFriendRequest(targetLoginField.text)
                targetLoginField.text = ""
            }
        }

        Text {
            text: "Друзья"
            font.pixelSize: 16
            font.bold: true
            color: "#212121"
            Layout.fillWidth: true
        }

        ListView {
            id: friendList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 100
            spacing: 6
            clip: true

            model: root.friends

            delegate: Row {
                width: friendList.width
                spacing: 8

                Text {
                    text: modelData
                    font.bold: true
                    font.pixelSize: 14
                    color: "#1565C0"
                }
            }
        }

        Text {
            text: "Ожидают ответа"
            font.pixelSize: 16
            font.bold: true
            color: "#212121"
            Layout.fillWidth: true
        }

        ListView {
            id: pendingList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumHeight: 100
            spacing: 6
            clip: true

            model: root.pending

            delegate: Row {
                width: pendingList.width
                spacing: 8

                Text {
                    text: modelData
                    font.bold: true
                    font.pixelSize: 14
                    color: "#1565C0"
                }

                Button {
                    Rectangle {
                        height: 50
                        width: 50
                        color: "#59ff00"
                    }

                    onClicked: {
                        ClientHandler.acceptFriendRequest(modelData)
                        ClientHandler.getFriendList()
                    }
                }

                Button {
                    height: 50
                    width: 50
                    background: Rectangle {
                        radius: 10
                        color: "#ff2a00"
                    }

                    onClicked: {
                        ClientHandler.rejectFriendRequest(modelData)
                        ClientHandler.getFriendList()
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