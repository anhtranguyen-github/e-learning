import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Window 2.15
import "."

Window {
    width: 1280
    height: 720
    visible: true
    title: "Socker Learning App"
    color: Style.backgroundColor

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: loginComponent
        
        pushEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        pushExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
        popEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to: 1
                duration: 200
            }
        }
        popExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to: 0
                duration: 200
            }
        }
    }

    Component {
        id: loginComponent
        Page {
            background: Rectangle { color: Style.backgroundColor }
            
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
                    statusMessage.color = Style.errorColor
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: 400
                height: contentLayout.height + 60
                color: Style.cardBackground
                radius: Style.cornerRadius
                
                // Simple shadow
                border.color: "#e0e0e0"
                border.width: 1

                ColumnLayout {
                    id: contentLayout
                    anchors.centerIn: parent
                    width: parent.width * 0.85
                    spacing: 20

                    Text {
                        text: "Welcome Back"
                        font.family: Style.fontFamily
                        font.pixelSize: 32
                        font.bold: true
                        color: Style.textColor
                        Layout.alignment: Qt.AlignHCenter
                    }

                    TextField {
                        id: usernameField
                        placeholderText: "Username"
                        Layout.fillWidth: true
                        font.pixelSize: Style.bodySize
                        background: Rectangle {
                            color: "#f0f2f5"
                            radius: Style.cornerRadius
                            border.color: usernameField.activeFocus ? Style.primaryColor : "transparent"
                            border.width: 2
                        }
                        padding: 12
                    }

                    TextField {
                        id: passwordField
                        placeholderText: "Password"
                        echoMode: TextInput.Password
                        Layout.fillWidth: true
                        font.pixelSize: Style.bodySize
                        background: Rectangle {
                            color: "#f0f2f5"
                            radius: Style.cornerRadius
                            border.color: passwordField.activeFocus ? Style.primaryColor : "transparent"
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
                            font.pixelSize: Style.bodySize
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        background: Rectangle {
                            color: parent.enabled ? Style.primaryColor : Style.secondaryColor
                            radius: Style.cornerRadius
                        }

                        onClicked: {
                            isBusy = true
                            statusMessage.text = "Connecting..."
                            statusMessage.color = Style.secondaryTextColor
                            networkManager.login(usernameField.text, passwordField.text)
                        }
                    }

                    Text {
                        id: statusMessage
                        text: ""
                        font.pixelSize: Style.smallSize
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
