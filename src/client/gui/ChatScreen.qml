import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Page {
    id: chatScreen
    title: "Chat"

    property string currentRecipient: ""
    property var chatModel: ListModel {}
    property var recentChatsModel: ListModel {}

    background: Rectangle {
        color: "#f5f5f5"
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
                                color: model.sender === "me" ? "#DCF8C6" : "#FFFFFF"
                                radius: 10
                                border.color: "#e0e0e0"
                                
                                Layout.maximumWidth: chatListView.width * 0.7
                                Layout.preferredWidth: msgContent.width + 20
                                Layout.preferredHeight: msgContent.height + 20

                                ColumnLayout {
                                    id: msgContent
                                    anchors.centerIn: parent
                                    spacing: 5

                                    Label {
                                        text: model.sender
                                        font.pixelSize: 10
                                        color: "#888888"
                                        visible: model.sender !== "me"
                                    }

                                    Loader {
                                        sourceComponent: model.type === "AUDIO" ? audioComponent : textComponent
                                        
                                        Component {
                                            id: textComponent
                                            Label {
                                                text: model.content
                                                wrapMode: Text.Wrap
                                                width: Math.min(implicitWidth, chatListView.width * 0.6)
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

    function sendMessage() {
        if (messageInput.text.trim() !== "") {
            networkManager.sendPrivateMessage(currentRecipient, messageInput.text, "TEXT")
        }
    }
}
