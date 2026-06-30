import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "DirectMessagePage"
    color: "#F4F6FB"

    property string peerLogin: ""

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // шапка
        Rectangle {
            Layout.fillWidth: true
            height: 56
            color: "#FFFFFF"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 16
                spacing: 8

                Button {
                    text: "←"
                    font.pixelSize: 18
                    flat: true
                    Layout.preferredWidth: 36
                    Layout.preferredHeight: 36
                    onClicked: stackView.pop()

                    contentItem: Text {
                        text: parent.text
                        color: "#6B7088"
                        font: parent.font
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Rectangle {
                    width: 34
                    height: 34
                    radius: 11
                    color: "#EEF0FE"

                    Text {
                        anchors.centerIn: parent
                        text: root.peerLogin.length > 0 ? root.peerLogin.charAt(0).toUpperCase() : "?"
                        color: "#5B6EF5"
                        font.bold: true
                        font.pixelSize: 14
                    }
                }

                Text {
                    text: root.peerLogin
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                    color: "#1A1D29"
                    Layout.fillWidth: true
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                width: parent.width
                height: 1
                color: "#EDEFF7"
            }
        }

        // сами сообщения
        ListView {
            id: messageList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 8
            Layout.rightMargin: 8
            clip: true
            spacing: 8

            footer: Item { height: 8 }
            header: Item { height: 8 }

            model: ListModel { id: messageModel }

            delegate: ColumnLayout {
                width: messageList.width
                spacing: 2

                // я справа кент слева
                property bool isMe: model.isMe

                Item {
                    Layout.fillWidth: true
                    height: bubble.height

                    Rectangle {
                        id: bubble
                        anchors.right: isMe ? parent.right : undefined
                        anchors.left:  isMe ? undefined : parent.left
                        anchors.rightMargin: 8
                        anchors.leftMargin:  8

                        width: Math.min(messageText.implicitWidth + 24, messageList.width * 0.7)
                        height: messageText.implicitHeight + 16
                        radius: 16
                        color: isMe ? "#5B6EF5" : "#FFFFFF"
                        border.width: isMe ? 0 : 1
                        border.color: "#EDEFF7"

                        Text {
                            id: messageText
                            anchors.fill: parent
                            anchors.margins: 12
                            text: model.message
                            color: isMe ? "#FFFFFF" : "#1A1D29"
                            font.pixelSize: 14
                            wrapMode: Text.Wrap
                        }
                    }
                }

                // время
                Text {
                    Layout.alignment: isMe ? Qt.AlignRight : Qt.AlignLeft
                    Layout.rightMargin: isMe ? 12 : 0
                    Layout.leftMargin:  isMe ? 0 : 12
                    text: formatTime(model.timestamp)
                    font.pixelSize: 11
                    color: "#A8ACBD"
                }
            }

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
                width: 6
                background: Rectangle {
                    color: "#E7E9F3"
                    radius: 3
                }
                contentItem: Rectangle {
                    color: "#5B6EF5"
                    radius: 3
                }
            }
        }

        // ввод сообщения
        Rectangle {
            Layout.fillWidth: true
            height: 64
            color: "#FFFFFF"

            Rectangle {
                anchors.top: parent.top
                width: parent.width
                height: 1
                color: "#EDEFF7"
            }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                TextField {
                    id: messageField
                    Layout.fillWidth: true
                    placeholderText: "Сообщение..."
                    font.pixelSize: 14
                    leftPadding: 14

                    background: Rectangle {
                        radius: 20
                        color: "#F4F6FB"
                        border.width: messageField.activeFocus ? 1.5 : 0
                        border.color: "#5B6EF5"
                    }
                    Keys.onReturnPressed: sendMessage()
                }

                Button {
                    text: "→"
                    font.pixelSize: 18
                    Layout.preferredWidth: 40
                    Layout.preferredHeight: 40
                    enabled: messageField.text.trim() !== ""
                    onClicked: sendMessage()

                    background: Rectangle {
                        radius: 20
                        color: parent.enabled ? "#5B6EF5" : "#D8DCEF"
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

    function scrollToBottom() {
        Qt.callLater(function() {
            messageList.positionViewAtEnd()
        })
        
    }

    function sendMessage() {
        var text = messageField.text.trim()
        if(text === "") return

        messageModel.append({
            message:   text,
            timestamp: Math.floor(Date.now() / 1000),
            isMe:      true
        })

        scrollToBottom();

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

        scrollToBottom()
    }

    Connections {
        target: ClientBridge

        function onDirectMessageReceived(senderLogin, message, timestamp) {
            if(senderLogin !== root.peerLogin) return
            messageModel.append({
                message:   message,
                timestamp: timestamp,
                isMe:      false
            })

            scrollToBottom()
        }

        function onHistoryReceived(messages) {
            loadHistory(messages)
        }
    }

    Component.onCompleted: {
        if(root.peerLogin !== "")
            ClientHandler.getHistoryDirectMessage(root.peerLogin)
    }
}