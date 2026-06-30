import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge 1.0
import Client.ErrorHandler 1.0

Rectangle {
    id: root
    objectName: "authScreen"
    color: "#F4F6FB"

    Rectangle {
        anchors.centerIn: parent
        width: 360
        height: cardContent.height + 64
        radius: 20
        color: "#FFFFFF"

        Rectangle {
            anchors.fill: parent
            anchors.topMargin: 6
            radius: parent.radius
            color: "#5B6EF5"
            opacity: 0.06
            z: -1
            scale: 1.02
        }

        ColumnLayout {
            id: cardContent
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: 32
            spacing: 16

            Rectangle {
                width: 52
                height: 52
                radius: 16
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 4
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop { position: 0.0; color: "#5B6EF5" }
                    GradientStop { position: 1.0; color: "#7C8CFF" }
                }

                Text {
                    anchors.centerIn: parent
                    text: "✦"
                    color: "#FFFFFF"
                    font.pixelSize: 22
                    font.bold: true
                }
            }

            Text {
                text: "Добро пожаловать"
                font.pixelSize: 19
                font.weight: Font.DemiBold
                color: "#1A1D29"
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "Войдите или создайте аккаунт"
                font.pixelSize: 13
                color: "#8A8FA3"
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 8
            }

            TextField {
                id: loginField
                Layout.fillWidth: true
                placeholderText: "Логин"
                font.pixelSize: 14
                leftPadding: 14
                topPadding: 12
                bottomPadding: 12

                background: Rectangle {
                    radius: 12
                    color: "#F7F8FC"
                    border.width: loginField.activeFocus ? 1.5 : 1
                    border.color: loginField.activeFocus ? "#5B6EF5" : "#E7E9F3"
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }

                onTextChanged: {
                    if(authErrorRect.visible)
                        authErrorRect.visible = false
                }
            }

            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: "Пароль"
                echoMode: TextInput.Password
                font.pixelSize: 14
                leftPadding: 14
                topPadding: 12
                bottomPadding: 12

                background: Rectangle {
                    radius: 12
                    color: "#F7F8FC"
                    border.width: passwordField.activeFocus ? 1.5 : 1
                    border.color: passwordField.activeFocus ? "#5B6EF5" : "#E7E9F3"
                    Behavior on border.color { ColorAnimation { duration: 120 } }
                }

                onTextChanged: {
                    if(authErrorRect.visible)
                        authErrorRect.visible = false
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Layout.topMargin: 4
                spacing: 10

                Button {
                    text: "Войти"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    onClicked: ClientHandler.loginUser(loginField.text, passwordField.text)

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
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Button {
                    text: "Регистрация"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 42
                    onClicked: ClientHandler.registerUser(loginField.text, passwordField.text)

                    background: Rectangle {
                        radius: 12
                        color: "#F7F8FC"
                        border.width: 1
                        border.color: "#E7E9F3"
                    }

                    contentItem: Text {
                        text: parent.text
                        color: "#5B6EF5"
                        font.pixelSize: 14
                        font.weight: Font.DemiBold
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Rectangle {
                id: authErrorRect
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                Layout.topMargin: 4
                color: "#FFF0F2"
                radius: 10
                visible: false

                Text {
                    id: authError
                    anchors.centerIn: parent
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    color: "#FF5C72"
                }
            }
        }
    }

    Connections {
        target: ClientBridge

        function onAuthResult(token, errorCode) {
            if(ClientErrorHandler.isOk(errorCode)) {
                stackView.replace("MainScreen.qml")
            }
            else {
                passwordField.text = ""
                // authError.text = "Ошибка: " + errorCode
                // authErrorRect.visible = true
                window.showToast(ClientErrorHandler.textError(errorCode), "error", 4000)
            }
        }
    }
}