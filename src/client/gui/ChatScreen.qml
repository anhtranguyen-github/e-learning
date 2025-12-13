import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    id: chatScreen
    title: "Chat"

    property string currentRecipient: ""
    property var chatModel: ListModel {}
    property var recentChatsModel: ListModel {}
    
    // Voice Call properties
    property bool inCall: false
    property string callPartner: ""
    property string incomingCaller: ""
    property string incomingCallerId: ""

    background: Rectangle {
        color: "#f5f5f5"
    }

    header: Header {
        title: "Chat"
        showBack: true
        onBackClicked: stackView.pop()
    }

    // Voice Call Connections
    Connections {
        target: networkManager
        
        function onIncomingCall(callerUsername, callerId) {
            incomingCaller = callerUsername
            incomingCallerId = callerId
            incomingCallDialog.open()
            addSystemMessage("Incoming Call from " + callerUsername)
        }
        
        function onCallAnswered(username) {
            inCall = true
            callPartner = username
            outgoingCallDialog.close()
            inCallDialog.open()
            addSystemMessage("Call Answered")
        }
        
        function onCallEnded(reason) {
            inCall = false
            callPartner = ""
            inCallDialog.close()
            incomingCallDialog.close()
            outgoingCallDialog.close()
            addSystemMessage("Call Ended: " + reason)
        }
        
        function onCallFailed(reason) {
            inCall = false
            callPartner = ""
            outgoingCallDialog.close()
            callFailedMessage.text = reason
            callFailedMessage.visible = true
            addSystemMessage("Call Failed: " + reason)
        }
    }

    // Incoming Call Dialog
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
                        inCallDialog.open()
                    }
                }

                Button {
                    text: "Ignore"
                    contentItem: Text {
                        text: parent.text
                        color: Style.textColor
                        horizontalAlignment: Text.AlignHCenter
                    }
                    background: Rectangle {
                        color: "#e0e0e0"
                        radius: Style.cornerRadius
                    }
                    onClicked: {
                        incomingCallDialog.close()
                    }
                }
            }
        }
    }

    // In-Call Dialog
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
                text: "(Voice call simulation - no actual audio)"
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

    // Outgoing Call Dialog
    Dialog {
        id: outgoingCallDialog
        title: "Calling..."
        modal: true
        closePolicy: Popup.NoAutoClose
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 300
        
        background: Rectangle {
            color: Style.cardBackground
            radius: Style.cornerRadius
            border.color: Style.primaryColor
            border.width: 2
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
                text: "Calling " + currentRecipient + "..."
                font.family: Style.fontFamily
                font.pixelSize: Style.subHeadingSize
                font.bold: true
                color: Style.textColor
                Layout.alignment: Qt.AlignHCenter
            }

            Button {
                text: "Cancel"
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
                    networkManager.endCall(currentRecipient)
                    outgoingCallDialog.close()
                }
            }
        }
    }

    // Call Failed Message
    Text {
        id: callFailedMessage
        visible: false
        text: ""
        color: Style.errorColor
        font.pixelSize: Style.smallSize
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 10
        
        Timer {
            running: callFailedMessage.visible
            interval: 3000
            onTriggered: callFailedMessage.visible = false
        }
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // Left Pane: Recent Chats
        Rectangle {
            Layout.preferredWidth: 300
            Layout.fillHeight: true
            color: "#ffffff"
            border.color: "#e0e0e0"
            border.width: 1

            ColumnLayout {
                anchors.fill: parent
                spacing: 0

                // Header
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "#f8f9fa"
                    border.color: "#e0e0e0"
                    border.width: 1

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        
                        Label {
                            text: "Chats"
                            font.bold: true
                            font.pixelSize: 16
                            Layout.fillWidth: true
                        }

                        Button {
                            text: "+"
                            onClicked: newChatDialog.open()
                        }
                    }
                }

                // List
                ListView {
                    id: recentChatsList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: recentChatsModel
                    delegate: ItemDelegate {
                        width: ListView.view.width
                        height: 60
                        
                        background: Rectangle {
                            color: (currentRecipient === model.username) ? "#e3f2fd" : (parent.hovered ? "#f5f5f5" : "#ffffff")
                        }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 10

                            Rectangle {
                                width: 40
                                height: 40
                                radius: 20
                                color: "#bdbdbd"
                                Label {
                                    anchors.centerIn: parent
                                    text: model.username.charAt(0).toUpperCase()
                                    color: "white"
                                    font.bold: true
                                }
                            }

                            ColumnLayout {
                                Layout.fillWidth: true
                                spacing: 2
                                
                                Label {
                                    text: model.username
                                    font.bold: true
                                }
                                Label {
                                    text: model.lastMessage
                                    color: "#757575"
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                    font.pixelSize: 12
                                }
                            }
                            
                            Label {
                                text: model.timestamp.split(' ')[1] || "" // Show time only
                                color: "#9e9e9e"
                                font.pixelSize: 10
                            }
                        }

                        onClicked: {
                            currentRecipient = model.username
                            networkManager.requestChatHistory(currentRecipient)
                        }
                    }
                }
            }
        }

        // Right Pane: Chat View
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: "#f5f5f5"

            ColumnLayout {
                anchors.fill: parent
                spacing: 10

                // Header
                Rectangle {
                    Layout.fillWidth: true
                    height: 50
                    color: "#ffffff"
                    border.color: "#e0e0e0"
                    visible: currentRecipient !== ""

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        Button {
                            text: "Back"
                            visible: chatScreen.width < 600 // Show back button on small screens
                            onClicked: currentRecipient = ""
                        }

                        Label {
                            text: currentRecipient
                            font.bold: true
                            font.pixelSize: 18
                            Layout.fillWidth: true
                            horizontalAlignment: Text.AlignHCenter
                        }

                        // Call Button
                        Button {
                            text: "📞 Call"
                            enabled: !inCall
                            contentItem: Text {
                                text: parent.text
                                color: parent.enabled ? "white" : Style.secondaryTextColor
                                font.pixelSize: Style.smallSize
                                horizontalAlignment: Text.AlignHCenter
                            }
                            background: Rectangle {
                                color: parent.enabled ? Style.primaryColor : "#e0e0e0"
                                radius: Style.cornerRadius
                            }
                            onClicked: {
                                networkManager.initiateCall(currentRecipient)
                                outgoingCallDialog.open()
                                addSystemMessage("Call Initiated")
                            }
                        }
                    }
                }

                // Chat History
                ListView {
                    id: chatListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: chatModel
                    clip: true
                    spacing: 10
                    visible: currentRecipient !== ""
                    
                    delegate: Item {
                        width: chatListView.width
                        height: msgRow.height

                        RowLayout {
                            id: msgRow
                            width: parent.width
                            layoutDirection: model.sender === "me" ? Qt.RightToLeft : Qt.LeftToRight
                            spacing: 10

                            Rectangle {
                                color: model.type === "SYSTEM" ? "#E0E0E0" : (model.sender === "me" ? "#DCF8C6" : "#FFFFFF")
                                radius: 10
                                border.color: "#e0e0e0"
                                
                                Layout.maximumWidth: chatListView.width * 0.7
                                Layout.preferredWidth: msgContent.width + 20
                                Layout.preferredHeight: msgContent.height + 20
                                Layout.alignment: model.type === "SYSTEM" ? Qt.AlignHCenter : (model.sender === "me" ? Qt.AlignRight : Qt.AlignLeft)

                                ColumnLayout {
                                    id: msgContent
                                    anchors.centerIn: parent
                                    spacing: 5

                                    Label {
                                        text: model.sender
                                        font.pixelSize: 10
                                        color: "#888888"
                                        visible: model.sender !== "me" && model.type !== "SYSTEM"
                                    }

                                    Loader {
                                        sourceComponent: model.type === "AUDIO" ? audioComponent : textComponent
                                        
                                        Component {
                                            id: textComponent
                                            Label {
                                                text: model.content
                                                wrapMode: Text.Wrap
                                                width: Math.min(implicitWidth, chatListView.width * 0.6)
                                                horizontalAlignment: model.type === "SYSTEM" ? Text.AlignHCenter : Text.AlignLeft
                                            }
                                        }

                                        Component {
                                            id: audioComponent
                                            RowLayout {
                                                Button {
                                                    text: "Play Audio (Not Available)"
                                                    enabled: false
                                                    onClicked: console.log("Audio playback requires QtMultimedia")
                                                }
                                            }
                                        }
                                    }

                                    Label {
                                        text: model.timestamp
                                        font.pixelSize: 10
                                        color: "#888888"
                                        Layout.alignment: Qt.AlignRight
                                        visible: model.type !== "SYSTEM"
                                    }
                                }
                            }
                        }
                    }
                }
                
                Label {
                    visible: currentRecipient === ""
                    text: "Select a chat to start messaging"
                    font.pixelSize: 16
                    color: "#757575"
                    Layout.alignment: Qt.AlignCenter
                }

                // Input Area
                Rectangle {
                    Layout.fillWidth: true
                    height: 60
                    color: "#ffffff"
                    border.color: "#e0e0e0"
                    visible: currentRecipient !== ""

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        TextField {
                            id: messageInput
                            Layout.fillWidth: true
                            placeholderText: "Type a message..."
                            onAccepted: sendMessage()
                        }

                        Button {
                            text: "Send"
                            onClicked: sendMessage()
                        }
                        
                        Button {
                            text: isRecording ? "Stop" : "Record"
                            property bool isRecording: false
                            onClicked: {
                                if (isRecording) {
                                    isRecording = false
                                    // Stop recording logic
                                } else {
                                    isRecording = true
                                    // Start recording logic
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Dialog {
        id: newChatDialog
        title: "New Chat"
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        ColumnLayout {
            spacing: 10
            Label { text: "Enter username:" }
            TextField {
                id: newChatUsername
                placeholderText: "Username"
            }
        }

        onAccepted: {
            if (newChatUsername.text !== "") {
                currentRecipient = newChatUsername.text
                networkManager.requestChatHistory(currentRecipient)
                // Optimistically add to list or wait for refresh
                // We'll wait for refresh or next message
                newChatUsername.text = ""
            }
        }
    }

    Connections {
        target: networkManager
        
        function onChatMessageReceived(sender, content, type, timestamp) {
            if (sender === currentRecipient || sender === "me") {
                chatModel.append({
                    "sender": sender,
                    "content": content,
                    "type": type,
                    "timestamp": timestamp
                })
                chatListView.positionViewAtEnd()
            }
            // Refresh recent chats to update last message
            networkManager.requestRecentChats()
        }

        function onChatMessageSent(content) {
            var now = new Date()
            var timestamp = now.toLocaleTimeString()
            
            chatModel.append({
                "sender": "me",
                "content": content,
                "type": "TEXT", // Simplified
                "timestamp": timestamp
            })
            chatListView.positionViewAtEnd()
            messageInput.text = ""
            
            // Refresh recent chats
            networkManager.requestRecentChats()
        }
        
        function onChatHistoryReceived(historyData) {
            chatModel.clear()
            var messages = historyData.split('|');
            for (var i = 0; i < messages.length; i++) {
                if (messages[i] === "") continue;
                var parts = messages[i].split(';');
                if (parts.length >= 4) {
                    var sender = parts[0];
                    var type = parts[1];
                    var content = parts[2];
                    var timestamp = parts[3];
                    
                    chatModel.append({
                        "sender": sender,
                        "content": content,
                        "type": type,
                        "timestamp": timestamp
                    })
                }
            }
            chatListView.positionViewAtEnd()
        }

        function onRecentChatsReceived(chatsData) {
            recentChatsModel.clear()
            var chats = chatsData.split('|');
            for (var i = 0; i < chats.length; i++) {
                if (chats[i] === "") continue;
                var parts = chats[i].split(';');
                if (parts.length >= 4) {
                    var userId = parts[0];
                    var username = parts[1];
                    var lastMessage = parts[2];
                    var timestamp = parts[3];
                    
                    recentChatsModel.append({
                        "userId": userId,
                        "username": username,
                        "lastMessage": lastMessage,
                        "timestamp": timestamp
                    })
                }
            }
        }
    }

    Component.onCompleted: {
        networkManager.requestRecentChats()
    }

    function addSystemMessage(text) {
        var now = new Date()
        var timestamp = now.toLocaleTimeString()
        
        chatModel.append({
            "sender": "System",
            "content": text,
            "type": "SYSTEM",
            "timestamp": timestamp
        })
        chatListView.positionViewAtEnd()
    }

    function sendMessage() {
        if (messageInput.text.trim() !== "") {
            networkManager.sendPrivateMessage(currentRecipient, messageInput.text, "TEXT")
        }
    }
}
