import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

Window {
    id: window
    visible: true
    title: "Тестинг да"

    color: "#fff"

    ColumnLayout {
        Layout.fillWidth: true
        Layout.preferredHeight: 80
        Text {
            text: "работает вроде"
            font.pixelSize: 14
            font.bold: true
            color: "#000"
        }
    }
}
