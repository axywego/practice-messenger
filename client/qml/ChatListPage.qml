import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "ChatListPage"
    color: "#F5F5F5"

    property var chatList: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8

        // Шапка с кнопкой нового чата
        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Сообщения"
                font.pixelSize: 18
                font.bold: true
                color: "#212121"
                Layout.fillWidth: true
            }

            Button {
                text: "＋"
                font.pixelSize: 18
                onClicked: newChatPopup.open()
            }
        }

        // Список чатов
        ListView {
            id: chatListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: root.chatList

            delegate: Rectangle {
                width: chatListView.width
                height: 64
                color: mouseArea.containsMouse ? "#EEEEEE" : "#FFFFFF"

                Behavior on color { ColorAnimation { duration: 100 } }

                Column {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 12
                    spacing: 4

                    Text {
                        text: modelData.peer ?? ""
                        font.bold: true
                        font.pixelSize: 14
                        color: "#212121"
                    }

                    Text {
                        text: modelData.message ?? ""
                        color: "#757575"
                        font.pixelSize: 12
                        elide: Text.ElideRight
                        width: root.width - 24
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    width: parent.width
                    height: 1
                    color: "#EEEEEE"
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        stackView.push("DirectMessagePage.qml", {
                            peerLogin: modelData.peer
                        })
                    }
                }
            }
        }
    }

    // Попап для нового чата
    Popup {
        id: newChatPopup
        anchors.centerIn: parent
        width: 280
        padding: 16
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 12
            color: "#FFFFFF"
            layer.enabled: true
            layer.effect: null
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 12

            Text {
                text: "Новый чат"
                font.pixelSize: 16
                font.bold: true
                color: "#212121"
            }

            TextField {
                id: newChatLogin
                Layout.fillWidth: true
                placeholderText: "Логин друга"

                // Показываем только друзей из списка
                Keys.onReturnPressed: startChat()
            }

            // Список друзей для быстрого выбора
            ListView {
                id: friendSuggestions
                Layout.fillWidth: true
                height: Math.min(contentHeight, 150)
                clip: true
                model: friendsFiltered

                delegate: Rectangle {
                    width: parent.width
                    height: 40
                    color: friendMouse.containsMouse ? "#F5F5F5" : "transparent"
                    radius: 6

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 8
                        text: modelData
                        font.pixelSize: 14
                        color: "#212121"
                    }

                    MouseArea {
                        id: friendMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            newChatLogin.text = modelData
                            startChat()
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    onClicked: {
                        newChatPopup.close()
                        newChatLogin.text = ""
                    }
                }

                Button {
                    text: "Открыть чат"
                    Layout.fillWidth: true
                    enabled: newChatLogin.text.trim() !== ""
                    onClicked: startChat()
                }
            }
        }
    }

    // Фильтрованный список друзей по введённому тексту
    property var allFriends: []
    property var friendsFiltered: {
        var query = newChatLogin.text.toLowerCase()
        if(query === "") return root.allFriends
        return root.allFriends.filter(function(f) {
            return f.toLowerCase().indexOf(query) !== -1
        })
    }

    function startChat() {
        var login = newChatLogin.text.trim()
        if(login === "") return
        newChatPopup.close()
        newChatLogin.text = ""
        stackView.push("DirectMessagePage.qml", {
            peerLogin: login
        })
    }

    Connections {
        target: ClientBridge

        function onChatListReceived(chats) {
            root.chatList = chats
        }

        function onFriendListReceived(friends, pending) {
            root.allFriends = friends
        }
    }

    Component.onCompleted: {
        ClientHandler.tokenReady.connect(onTokenReady)
        if(ClientHandler.isTokenReady) {
            ClientHandler.getLastMessages()
            ClientHandler.getFriendList()  // загружаем друзей для подсказок
        }
    }

    function onTokenReady() {
        ClientHandler.getLastMessages()
        ClientHandler.getFriendList()
    }
}