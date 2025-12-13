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

    // Global Call Properties
    property string incomingCaller: ""
    property string incomingCallerId: ""
    property string callPartner: ""
    property bool inCall: false

    // Global Call Handling
    Connections {
        target: networkManager
        
        function onIncomingCall(callerUsername, callerId) {
            incomingCaller = callerUsername
            incomingCallerId = callerId
            incomingCallDialog.open()
            incomingCallTimer.restart()
        }
        
        function onCallAnswered(username) {
            inCall = true
            callPartner = username
            incomingCallDialog.close()
            incomingCallTimer.stop()
            inCallDialog.open()
        }
        
        function onCallEnded(reason) {
            inCall = false
            callPartner = ""
            inCallDialog.close()
            incomingCallDialog.close()
            incomingCallTimer.stop()
        }
        
        function onCallFailed(reason) {
            inCall = false
            callPartner = ""
            incomingCallDialog.close()
            incomingCallTimer.stop()
        }
    }

    // Incoming Call Dialog (Global)
    Dialog {
        id: incomingCallDialog
        title: "Incoming Call"
        modal: true
        closePolicy: Popup.NoAutoClose
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 350
        
        background: Rectangle {
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: Style.primaryColor
            border.width: 2
        }

        Timer {
            id: incomingCallTimer
            interval: 5000 // 5 seconds timeout
            repeat: false
            onTriggered: {
                if (incomingCallDialog.visible) {
                    networkManager.declineCall(incomingCaller)
                    incomingCallDialog.close()
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 20

            Text {
                text: "📞"
                font.pixelSize: 48
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: incomingCaller + " is calling..."
                font.family: Style.fontFamily
                font.pixelSize: Style.subHeadingSize
                font.bold: true
                color: Style.textColor
                Layout.alignment: Qt.AlignHCenter
            }

            RowLayout {
                Layout.alignment: Qt.AlignHCenter
                spacing: 20

                Button {
                    text: "Decline"
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        color: Style.errorColor
                        radius: Style.cornerRadius
                    }
                    onClicked: {
                        networkManager.declineCall(incomingCaller)
                        incomingCallDialog.close()
                        incomingCallTimer.stop()
                    }
                }

                Button {
                    text: "Answer"
                    contentItem: Text {
                        text: parent.text
                        color: "white"
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        color: Style.successColor
                        radius: Style.cornerRadius
                    }
                    onClicked: {
                        networkManager.answerCall(incomingCaller)
                        inCall = true
                        callPartner = incomingCaller
                        incomingCallDialog.close()
                        incomingCallTimer.stop()
                        inCallDialog.open()
                    }
                }
            }
        }
    }

    // In-Call Dialog (Global)
    Dialog {
        id: inCallDialog
        title: "In Call"
        modal: true
        closePolicy: Popup.NoAutoClose
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 300

        background: Rectangle {
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: Style.successColor
            border.width: 2
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: 20

            Text {
                text: "🔊"
                font.pixelSize: 48
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "In call with " + callPartner
                font.family: Style.fontFamily
                font.pixelSize: Style.bodySize
                font.bold: true
                color: Style.textColor
                Layout.alignment: Qt.AlignHCenter
            }

            Text {
                text: "(Voice call simulation)"
                font.pixelSize: Style.smallSize
                color: Style.secondaryTextColor
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: "End Call"
                Layout.alignment: Qt.AlignHCenter
                contentItem: Text {
                    text: parent.text
                    color: "white"
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
                background: Rectangle {
                    color: Style.errorColor
                    radius: Style.cornerRadius
                }
                onClicked: {
                    networkManager.endCall(callPartner)
                    inCall = false
                    callPartner = ""
                    inCallDialog.close()
                }
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

                    // Register link
                    Text {
                        text: "Don't have an account? <a href='#'>Register</a>"
                        font.pixelSize: Style.smallSize
                        color: Style.secondaryTextColor
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.RichText
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: stackView.push(registerComponent)
                        }
                    }
                }
            }
        }
    }

    Component {
        id: registerComponent
        Page {
            background: Rectangle { color: Style.backgroundColor }
            
            property bool isBusy: false

            Connections {
                target: networkManager
                function onRegisterSuccess() {
                    isBusy = false
                    regStatusMessage.text = "Registration successful! Please login."
                    regStatusMessage.color = Style.successColor
                    // Go back to login after short delay
                    successTimer.start()
                }
                function onRegisterFailure(message) {
                    isBusy = false
                    regStatusMessage.text = message
                    regStatusMessage.color = Style.errorColor
                }
            }

            Timer {
                id: successTimer
                interval: 1500
                repeat: false
                onTriggered: stackView.pop()
            }

            Rectangle {
                anchors.centerIn: parent
                width: 400
                height: regContentLayout.height + 60
                color: Style.cardBackground
                radius: Style.cornerRadius
                
                border.color: "#e0e0e0"
                border.width: 1

                ColumnLayout {
                    id: regContentLayout
                    anchors.centerIn: parent
                    width: parent.width * 0.85
                    spacing: 20

                    Text {
                        text: "Create Account"
                        font.family: Style.fontFamily
                        font.pixelSize: 32
                        font.bold: true
                        color: Style.textColor
                        Layout.alignment: Qt.AlignHCenter
                    }

                    TextField {
                        id: regUsernameField
                        placeholderText: "Username (min 3 characters)"
                        Layout.fillWidth: true
                        font.pixelSize: Style.bodySize
                        background: Rectangle {
                            color: "#f0f2f5"
                            radius: Style.cornerRadius
                            border.color: regUsernameField.activeFocus ? Style.primaryColor : "transparent"
                            border.width: 2
                        }
                        padding: 12
                    }

                    TextField {
                        id: regPasswordField
                        placeholderText: "Password (min 4 characters)"
                        echoMode: TextInput.Password
                        Layout.fillWidth: true
                        font.pixelSize: Style.bodySize
                        background: Rectangle {
                            color: "#f0f2f5"
                            radius: Style.cornerRadius
                            border.color: regPasswordField.activeFocus ? Style.primaryColor : "transparent"
                            border.width: 2
                        }
                        padding: 12
                    }

                    TextField {
                        id: regConfirmPasswordField
                        placeholderText: "Confirm Password"
                        echoMode: TextInput.Password
                        Layout.fillWidth: true
                        font.pixelSize: Style.bodySize
                        background: Rectangle {
                            color: "#f0f2f5"
                            radius: Style.cornerRadius
                            border.color: regConfirmPasswordField.activeFocus ? Style.primaryColor : "transparent"
                            border.width: 2
                        }
                        padding: 12
                    }

                    Button {
                        text: isBusy ? "Registering..." : "Register"
                        Layout.fillWidth: true
                        enabled: !isBusy && regUsernameField.text.length >= 3 && regPasswordField.text.length >= 4 && regPasswordField.text === regConfirmPasswordField.text
                        
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
                            if (regPasswordField.text !== regConfirmPasswordField.text) {
                                regStatusMessage.text = "Passwords do not match"
                                regStatusMessage.color = Style.errorColor
                                return
                            }
                            isBusy = true
                            regStatusMessage.text = "Registering..."
                            regStatusMessage.color = Style.secondaryTextColor
                            networkManager.registerUser(regUsernameField.text, regPasswordField.text)
                        }
                    }

                    Text {
                        id: regStatusMessage
                        text: ""
                        font.pixelSize: Style.smallSize
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        wrapMode: Text.WordWrap
                        Layout.fillWidth: true
                    }

                    // Back to login link
                    Text {
                        text: "Already have an account? <a href='#'>Login</a>"
                        font.pixelSize: Style.smallSize
                        color: Style.secondaryTextColor
                        Layout.alignment: Qt.AlignHCenter
                        horizontalAlignment: Text.AlignHCenter
                        textFormat: Text.RichText
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: stackView.pop()
                        }
                    }
                }
            }
        }
    }
}
