import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "DirectMessagePage"
    color: "#F5F5F5"

    property string peerLogin: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // Шапка
        Rectangle {
            Layout.fillWidth: true
            height: 52
            color: "#FFFFFF"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 12
                spacing: 8

                Button {
                    text: "←"
                    font.pixelSize: 18
                    flat: true
                    onClicked: stackView.pop()
                }

                Text {
                    text: root.peerLogin
                    font.pixelSize: 16
                    font.bold: true
                    color: "#212121"
                    Layout.fillWidth: true
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#EEEEEE"
            }
        }

        // Список сообщений
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8

            footer: Item { height: 8 }
            header: Item { height: 8 }

            model: ListModel { id: messageModel }

            onCountChanged: positionViewAtEnd()

            delegate: ColumnLayout {
                width: messageList.width
                spacing: 2

                // Моё сообщение — справа, чужое — слева
                property bool isMe: model.isMe

                Item {
                    Layout.fillWidth: true
                    height: bubble.height

                    Rectangle {
                        id: bubble
                        anchors.right: isMe ? parent.right : undefined
                        anchors.left:  isMe ? undefined : parent.left
                        anchors.rightMargin: 12
                        anchors.leftMargin:  12

                        width: Math.min(messageText.implicitWidth + 24, messageList.width * 0.75)
                        height: messageText.implicitHeight + 16
                        radius: 16
                        color: isMe ? "#1565C0" : "#FFFFFF"

                        Text {
                            id: messageText
                            anchors.fill: parent
                            anchors.margins: 12
                            text: model.message
                            color: isMe ? "#FFFFFF" : "#212121"
                            font.pixelSize: 14
                            wrapMode: Text.Wrap
                        }
                    }
                }

                // Время
                Text {
                    Layout.alignment: isMe ? Qt.AlignRight : Qt.AlignLeft
                    Layout.rightMargin: isMe ? 16 : 0
                    Layout.leftMargin:  isMe ? 0 : 16
                    text: formatTime(model.timestamp)
                    font.pixelSize: 11
                    color: "#9E9E9E"
                }
            }
        }

        // Поле ввода
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: "#FFFFFF"

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#EEEEEE"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8

                TextField {
                    id: messageField
                    Layout.fillWidth: true
                    placeholderText: "Сообщение..."
                    background: Rectangle {
                        radius: 20
                        color: "#F5F5F5"
                    }
                    Keys.onReturnPressed: sendMessage()
                }

                Button {
                    text: "→"
                    font.pixelSize: 18
                    enabled: messageField.text.trim() !== ""
                    onClicked: sendMessage()

                    background: Rectangle {
                        radius: 20
                        color: parent.enabled ? "#1565C0" : "#BDBDBD"
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "#FFFFFF"
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }
        }
    }

    function sendMessage() {
        var text = messageField.text.trim()
        if(text === "") return

        // Добавляем своё сообщение сразу — не ждём ответа сервера
        messageModel.append({
            message:   text,
            timestamp: Math.floor(Date.now() / 1000),
            isMe:      true
        })

        ClientHandler.sendDirectMessage(root.peerLogin, text)
        messageField.text = ""
    }

    function formatTime(timestamp) {
        var date = new Date(timestamp * 1000)
        var h = date.getHours().toString().padStart(2, "0")
        var m = date.getMinutes().toString().padStart(2, "0")
        return h + ":" + m
    }

    function loadHistory(history) {
        messageModel.clear()
        for(var i = 0; i < history.length; i++) {
            var m = history[i]
            messageModel.append({
                message:   m.message,
                timestamp: m.timestamp,
                isMe:      m.from !== root.peerLogin
            })
        }
    }

    Connections {
        target: ClientBridge

        // Входящее личное сообщение в реальном времени
        function onDirectMessageReceived(senderLogin, message, timestamp) {
            if(senderLogin !== root.peerLogin) return // не наш чат
            messageModel.append({
                message:   message,
                timestamp: timestamp,
                isMe:      false
            })
        }

        // История переписки пришла
        function onHistoryReceived(messages) {
            loadHistory(messages)
        }
    }

    Component.onCompleted: {
        if(root.peerLogin !== "")
            ClientHandler.getHistoryDirectMessage(root.peerLogin)
    }
}