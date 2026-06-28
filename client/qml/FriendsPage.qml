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

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        Text {
            text: "Друзья"
            font.pixelSize: 16
            font.bold: true
            color: "#212121"
            Layout.fillWidth: true
        }

        ListView {
            id: chatList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6
            clip: true

            model: friends

            delegate: Row {
                width: chatList.width
                spacing: 8

                Text {
                    text: model.sender + ":"
                    font.bold: true
                    font.pixelSize: 14
                    color: "#1565C0"
                }

                Text {
                    text: model.message
                    font.pixelSize: 14
                    color: "#212121"
                    wrapMode: Text.Wrap
                    width: chatList.width - 120
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: messageField
                Layout.fillWidth: true
                placeholderText: "Написать сообщение..."
                Keys.onReturnPressed: sendMessage()
            }

            Button {
                text: "Отправить"
                onClicked: sendMessage()
            }
        }
    }

    function sendMessage() {
        if(messageField.text.trim() === "") return
        chatModel.append({ sender: "Я", message: messageField.text })
        ClientHandler.sendMessageGeneralChat(messageField.text)
        messageField.text = ""
    }

    Connections {
        target: ClientBridge

        function onChatMessageReceived(senderLogin, message) {
            chatModel.append({ sender: senderLogin, message: message })
        }
    }
}