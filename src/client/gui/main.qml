import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15

Window {
    width: 400
    height: 600
    visible: true
    title: "Socker Learning App"
    color: "#f0f2f5"

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: loginComponent
    }

    Component {
        id: loginComponent
        Page {
            title: "Login"
            
            property bool isBusy: false

            Connections {
                target: networkManager
                function onLoginSuccess() {
                    isBusy = false
                    stackView.push("Dashboard.qml")
                }
                function onLoginFailure(message) {
                    isBusy = false
                    statusMessage.text = message
                    statusMessage.color = "red"
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                width: parent.width * 0.8
                spacing: 20

                Text {
                    text: "Welcome Back"
                    font.pixelSize: 32
                    font.bold: true
                    color: "#1a1a1a"
                    Layout.alignment: Qt.AlignHCenter
                }

                TextField {
                    id: usernameField
                    placeholderText: "Username"
                    Layout.fillWidth: true
                    font.pixelSize: 16
                    background: Rectangle {
                        color: "white"
                        radius: 8
                        border.color: usernameField.activeFocus ? "#007bff" : "#e0e0e0"
                        border.width: 2
                    }
                    padding: 12
                }

                TextField {
                    id: passwordField
                    placeholderText: "Password"
                    echoMode: TextInput.Password
                    Layout.fillWidth: true
                    font.pixelSize: 16
                    background: Rectangle {
                        color: "white"
                        radius: 8
                        border.color: passwordField.activeFocus ? "#007bff" : "#e0e0e0"
                        border.width: 2
                    }
                    padding: 12
                }

                Button {
                    text: isBusy ? "Logging in..." : "Login"
                    Layout.fillWidth: true
                    enabled: !isBusy && usernameField.text.length > 0 && passwordField.text.length > 0
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }

                    background: Rectangle {
                        color: parent.enabled ? "#007bff" : "#cccccc"
                        radius: 8
                    }

                    onClicked: {
                        isBusy = true
                        statusMessage.text = "Connecting..."
                        statusMessage.color = "gray"
                        networkManager.login(usernameField.text, passwordField.text)
                    }
                }

                Text {
                    id: statusMessage
                    text: ""
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }
    }
}
