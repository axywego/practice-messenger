import QtQuick
import QtQuick.Controls
import Client.Handler 1.0
import Client.Bridge  1.0

Window {
    id: window
    width: 800
    height: 600
    visible: true
    title: "Мессенджер"

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: loadingComponent
    }

    Component {
        id: loadingComponent
        Rectangle {
            color: "#F5F5F5"
            Column {
                anchors.centerIn: parent
                spacing: 16
                BusyIndicator { anchors.horizontalCenter: parent.horizontalCenter }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Проверяем авторизацию..."
                    color: "#757575"
                }
            }
        }
    }

    Component.onCompleted: {
        if(!ClientHandler.checkSavedToken()) {
            stackView.replace("AuthScreen.qml")
        }
        else {
            tokenCheckTimer.start()
        }
    }

    Timer {
        id: tokenCheckTimer
        interval: 5000
        repeat: false

        onTriggered: {
            stackView.replace("AuthScreen.qml")
        }
    }

    Connections {
        target: ClientBridge

        function onTokenVerify(errorCode) {
            tokenCheckTimer.stop()

            console.log("ну и хули не регаешь")
            
            if(errorCode === 1) {
                console.log("токен норм")
                stackView.replace("MainScreen.qml")
            } 
            else {
                console.log("токен хуйня")
                stackView.replace("AuthScreen.qml")
            }
        }
    }
}