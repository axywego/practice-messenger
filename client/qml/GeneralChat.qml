import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "GeneralChat"
    color: "#F4F6FB"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        // шапочка
        Rectangle {
            Layout.fillWidth: true
            height: 56
            color: "#FFFFFF"

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                spacing: 10

                Rectangle {
                    width: 34
                    height: 34
                    radius: 11
                    color: "#EEF0FE"

                    Text {
                        anchors.centerIn: parent
                        text: "💬"
                        font.pixelSize: 16
                    }
                }

                Text {
                    text: "Общий чат"
                    font.pixelSize: 16
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

        ListView {
            id: chatList
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: 16
            spacing: 10
            clip: true

            model: ListModel { id: chatModel }

            onCountChanged: positionViewAtEnd()

            delegate: Item {
                width: chatList.width
                height: bubbleCol.height

                ColumnLayout {
                    id: bubbleCol
                    width: parent.width
                    spacing: 3

                    RowLayout {
                        spacing: 8
                        Layout.fillWidth: false

                        Rectangle {
                            width: 22
                            height: 22
                            radius: 7
                            color: "#5B6EF5"

                            Text {
                                anchors.centerIn: parent
                                text: model.sender.length > 0 ? model.sender.charAt(0).toUpperCase() : "?"
                                color: "#FFFFFF"
                                font.pixelSize: 11
                                font.bold: true
                            }
                        }

                        Text {
                            text: model.sender
                            font.bold: true
                            font.pixelSize: 13
                            color: "#5B6EF5"
                        }
                    }

                    Rectangle {
                        Layout.leftMargin: 30
                        Layout.preferredWidth: Math.min(messageText.implicitWidth + 24, chatList.width - 30)
                        Layout.preferredHeight: messageText.implicitHeight + 18
                        radius: 14
                        color: "#FFFFFF"
                        border.width: 1
                        border.color: "#EDEFF7"

                        Text {
                            id: messageText
                            anchors.fill: parent
                            anchors.margins: 10
                            text: model.message
                            font.pixelSize: 14
                            color: "#1A1D29"
                            wrapMode: Text.Wrap
                        }
                    }
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
                    placeholderText: "Написать сообщение..."
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