import QtQuick
import QtQuick.Controls
import Client.Handler 1.0
import Client.Bridge  1.0
import Client.ErrorHandler 1.0

Window {
    id: window
    width: 800
    height: 600
    visible: true
    title: "Мессенджер"
    color: "#F4F6FB"

    StackView {
        id: stackView
        anchors.fill: parent

        initialItem: loadingComponent
    }

    Column {
        id: toastContainer
        anchors.top: parent.top
        anchors.topMargin: 30
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        z: 999
        width: 400
    }

    function showToast(message, type, duration) {
        var component = Qt.createComponent("Toast.qml")
        if (component.status === Component.Ready) {
            var toast = component.createObject(toastContainer, {
                message: message,
                toastType: type || "info",
                duration: duration || 3000
            })
            toast.dismissed.connect(function() {
                toast.destroy()
            })
        }
    }

    Component {
        id: loadingComponent
        Rectangle {
            color: "#F4F6FB"

            Column {
                anchors.centerIn: parent
                spacing: 20

                Rectangle {
                    width: 56
                    height: 56
                    radius: 17
                    anchors.horizontalCenter: parent.horizontalCenter
                    gradient: Gradient {
                        orientation: Gradient.Vertical
                        GradientStop { position: 0.0; color: "#5B6EF5" }
                        GradientStop { position: 1.0; color: "#7C8CFF" }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "✦"
                        color: "#FFFFFF"
                        font.pixelSize: 24
                        font.bold: true
                    }
                }

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    palette.dark: "#5B6EF5"
                }

                Text {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: "Проверяем авторизацию..."
                    font.pixelSize: 13
                    color: "#8A8FA3"
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

            if(ClientErrorHandler.isOk(errorCode)) {
                stackView.replace("MainScreen.qml")
            }
            else {
                stackView.replace("AuthScreen.qml")
                window.showToast(ClientErrorHandler.textError(errorCode), "error", 3000)
            }
        }
    }
}