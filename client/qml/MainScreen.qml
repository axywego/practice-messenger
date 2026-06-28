import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "MainScreen"
    color: "#F5F5F5"

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Шапка с кнопкой выхода
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Общий чат"
                font.pixelSize: 16
                font.bold: true
                color: "#212121"
                Layout.fillWidth: true
            }

            Button {
                text: "Выйти"
                onClicked: {
                    ClientHandler.logout()
                    stackView.replace("AuthScreen.qml")
                }
            }
        }

        // Список сообщений
        ListView {
            id: chatList
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 6
            clip: true

            model: ListModel { id: chatModel }

            onCountChanged: positionViewAtEnd()

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

        // Поле ввода + кнопка отправки
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