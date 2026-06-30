import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge  1.0

Rectangle {
    id: root
    objectName: "ChatListPage"
    color: "#F4F6FB"

    property var chatList: []
    property var allFriends: []

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        RowLayout {
            Layout.fillWidth: true

            Text {
                text: "Сообщения"
                font.pixelSize: 20
                font.weight: Font.DemiBold
                color: "#1A1D29"
                Layout.fillWidth: true
            }

            Button {
                text: "+"
                font.pixelSize: 18
                Layout.preferredWidth: 38
                Layout.preferredHeight: 38

                background: Rectangle {
                    radius: 12
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: "#5B6EF5" }
                        GradientStop { position: 1.0; color: "#7C8CFF" }
                    }
                }

                contentItem: Text {
                    text: parent.text
                    color: "#FFFFFF"
                    font: parent.font
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: {
                    newChatLogin.text = ""
                    updateSuggestions("")
                    newChatPopup.open()
                }
            }
        }

        ListView {
            id: chatListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 8
            model: root.chatList

            delegate: Rectangle {
                width: chatListView.width
                height: 68
                radius: 14
                color: mouseArea.containsMouse ? "#F7F8FE" : "#FFFFFF"
                border.width: 1
                border.color: "#EDEFF7"

                Behavior on color { ColorAnimation { duration: 100 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 12

                    Rectangle {
                        width: 40
                        height: 40
                        radius: 13
                        color: "#EEF0FE"

                        Text {
                            anchors.centerIn: parent
                            text: (modelData.peer ?? "?").length > 0 ? (modelData.peer ?? "?").charAt(0).toUpperCase() : "?"
                            color: "#5B6EF5"
                            font.bold: true
                            font.pixelSize: 15
                        }
                    }

                    Column {
                        Layout.fillWidth: true
                        spacing: 4

                        Text {
                            text: modelData.peer ?? ""
                            font.bold: true
                            font.pixelSize: 14
                            color: "#1A1D29"
                        }

                        Text {
                            text: modelData.message ?? ""
                            color: "#8A8FA3"
                            font.pixelSize: 12
                            elide: Text.ElideRight
                            width: chatListView.width - 90
                        }
                    }
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

    Popup {
        id: newChatPopup
        anchors.centerIn: parent
        width: 300
        padding: 20
        modal: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            radius: 16
            color: "#FFFFFF"
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 14

            Text {
                text: "Новый чат"
                font.pixelSize: 17
                font.weight: Font.DemiBold
                color: "#1A1D29"
            }

            TextField {
                id: newChatLogin
                Layout.fillWidth: true
                placeholderText: "Логин друга"
                font.pixelSize: 14
                leftPadding: 12

                background: Rectangle {
                    radius: 10
                    color: "#F4F6FB"
                    border.width: newChatLogin.activeFocus ? 1.5 : 0
                    border.color: "#5B6EF5"
                }

                Keys.onReturnPressed: startChat()

                onTextChanged: updateSuggestions(text)
            }

            ListModel { id: suggestionsModel }

            ListView {
                id: friendSuggestions
                Layout.fillWidth: true
                height: Math.min(contentHeight, 150)
                clip: true
                model: suggestionsModel
                visible: suggestionsModel.count > 0

                delegate: Rectangle {
                    width: friendSuggestions.width
                    height: 40
                    color: friendMouse.containsMouse ? "#EEF0FE" : "transparent"
                    radius: 8

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        text: model.name
                        font.pixelSize: 14
                        color: "#1A1D29"
                    }

                    MouseArea {
                        id: friendMouse
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            newChatLogin.text = model.name
                            startChat()
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Button {
                    text: "Отмена"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    onClicked: {
                        newChatPopup.close()
                        newChatLogin.text = ""
                    }

                    background: Rectangle {
                        radius: 10
                        color: "#F4F6FB"
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "#6B7088"
                        font.pixelSize: 13
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: "Открыть чат"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    enabled: newChatLogin.text.trim() !== ""
                    onClicked: startChat()

                    background: Rectangle {
                        radius: 10
                        color: parent.enabled ? "transparent" : "#D8DCEF"
                        gradient: parent.enabled ? buttonGradient : null

                        Gradient {
                            id: buttonGradient
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
                    }
                }
            }
        }
    }

    function updateSuggestions(query) {
        suggestionsModel.clear()
        var q = query.toLowerCase()
        for(var i = 0; i < root.allFriends.length; i++) {
            var f = root.allFriends[i]
            if(q === "" || f.toLowerCase().indexOf(q) !== -1) {
                suggestionsModel.append({ name: f })
            }
        }
    }

    function startChat() {
        var login = newChatLogin.text.trim()
        if(login === "") return
        else if(!root.allFriends.includes(login)) return
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
            updateSuggestions(newChatLogin.text)
        }

        function onDirectMessageReceived(senderLogin, message, timestamp) {
            ClientHandler.getLastMessages()
        }
    }

    Connections {
        target: ClientHandler

        function onMessageSent(peer, message, timestamp) {
            ClientHandler.getLastMessages()
        }
    }

    Component.onCompleted: {
        ClientHandler.tokenReady.connect(onTokenReady)
        if(ClientHandler.isTokenReady) {
            ClientHandler.getLastMessages()
            ClientHandler.getFriendList()
        }
    }

    onVisibleChanged: {
        if(visible) ClientHandler.getLastMessages()
    }

    function onTokenReady() {
        ClientHandler.getLastMessages()
        ClientHandler.getFriendList()
    }
}