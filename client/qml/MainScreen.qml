import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0
import Client.UI 1.0

Rectangle {
    id: root
    objectName: "MainScreen"
    color: "#F5F5F5"

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Sidebar {
            id: sidebar

            Layout.fillHeight: true
            Layout.preferredWidth: 200
        }

        ColumnLayout {
            StackLayout {
                id: mainStack
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: sidebar.currentIndex

                GeneralChat { }
                FriendsPage { }
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