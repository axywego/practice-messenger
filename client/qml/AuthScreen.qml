import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Client.Handler 1.0
import Client.Bridge 1.0

Rectangle {
    id: root
    objectName: "authScreen"

    ColumnLayout {
        anchors.centerIn: parent
        width: 300
        spacing: 12

        TextField {
            id: loginField
            Layout.fillWidth: true
            placeholderText: "Логин"
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
            onTextChanged: {
                if(authErrorRect.visible)
                    authErrorRect.visible = false
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                text: "Войти"
                Layout.fillWidth: true
                onClicked: ClientHandler.loginUser(loginField.text, passwordField.text)
            }

            Button {
                text: "Зарегистрироваться"
                Layout.fillWidth: true
                onClicked: ClientHandler.registerUser(loginField.text, passwordField.text)
            }
        }

        Rectangle {
            id: authErrorRect
            Layout.fillWidth: true
            Layout.preferredHeight: 30
            color: "#FFEBEE"
            radius: 6
            visible: false

            Text {
                id: authError
                anchors.centerIn: parent
                font.pixelSize: 13
                font.weight: Font.DemiBold
                color: "#F44336"
            }
        }
    }

    // Слушаем ошибки авторизации — успех обрабатывается в Main.qml
    Connections {
        target: ClientBridge

        function onAuthResult(token, errorCode) {
            if(!errorCode == 1) {
                passwordField.text = ""
                authError.text = "Ошибка: " + errorCode
                authErrorRect.visible = true
            }
        }
    }
}