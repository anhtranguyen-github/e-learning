import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }

    property var gameModel: ListModel {}

    Connections {
        target: networkManager
        
        function onGameCreateSuccess(message) {
            statusMessage.text = "Game created successfully!"
            statusMessage.color = Style.successColor
            // Clear inputs
            typeField.currentIndex = 0
            levelField.currentIndex = 0
            jsonField.text = ""
        }

        function onGameCreateFailure(message) {
            statusMessage.text = "Error: " + message
            statusMessage.color = Style.errorColor
        }
        
        function onGameUpdateSuccess(message) {
            statusMessage.text = "Game updated successfully!"
            statusMessage.color = Style.successColor
        }

        function onGameUpdateFailure(message) {
            statusMessage.text = "Error: " + message
            statusMessage.color = Style.errorColor
        }

        function onGameDeleteSuccess(message) {
            statusMessage.text = "Game deleted successfully!"
            statusMessage.color = Style.successColor
            deleteIdField.text = ""
        }

        function onGameDeleteFailure(message) {
            statusMessage.text = "Error: " + message
            statusMessage.color = Style.errorColor
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 20

            Button {
                text: "← Back"
                background: Rectangle {
                    color: "transparent"
                    border.color: Style.primaryColor
                    border.width: 1
                    radius: Style.cornerRadius
                }
                contentItem: Text {
                    text: parent.text
                    color: Style.primaryColor
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                onClicked: stackView.pop()
            }

            Text {
                text: "Game Management"
                font.family: Style.fontFamily
                font.pixelSize: Style.headingSize
                font.bold: true
                color: Style.textColor
                Layout.fillWidth: true
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: parent.width
                spacing: 30

                // Create Game Section
                Rectangle {
                    Layout.fillWidth: true
                    height: createLayout.height + 40
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    
                    ColumnLayout {
                        id: createLayout
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        Text {
                            text: "Create / Update Game"
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.textColor
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            Text { text: "ID (Update only):"; color: Style.textColor; width: 100 }
                            TextField { 
                                id: gameIdField
                                placeholderText: "Leave empty for create"
                                Layout.fillWidth: true 
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            Text { text: "Type:"; color: Style.textColor; width: 100 }
                            ComboBox {
                                id: typeField
                                Layout.fillWidth: true
                                model: ["sentence_match", "word_match", "image_match"]
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            Text { text: "Level:"; color: Style.textColor; width: 100 }
                            ComboBox {
                                id: levelField
                                Layout.fillWidth: true
                                model: ["beginner", "intermediate", "advanced"]
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            
                            Text { text: "Question JSON:"; color: Style.textColor }
                            TextArea {
                                id: jsonField
                                Layout.fillWidth: true
                                Layout.preferredHeight: 150
                                placeholderText: '[{"question": "..."}]'
                                wrapMode: TextEdit.Wrap
                                background: Rectangle {
                                    border.color: "#ccc"
                                    radius: 4
                                }
                            }
                        }

                        RowLayout {
                            Layout.alignment: Qt.AlignRight
                            spacing: 10

                            Button {
                                text: "Create New"
                                background: Rectangle {
                                    color: Style.successColor
                                    radius: Style.cornerRadius
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    statusMessage.text = "Sending request..."
                                    statusMessage.color = Style.secondaryTextColor
                                    networkManager.requestAdminCreateGame(
                                        typeField.currentText,
                                        levelField.currentText,
                                        jsonField.text
                                    )
                                }
                            }

                            Button {
                                text: "Update Existing"
                                enabled: gameIdField.text.length > 0
                                background: Rectangle {
                                    color: style.enabled ? Style.primaryColor : Style.secondaryColor
                                    property var style: parent
                                    radius: Style.cornerRadius
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    statusMessage.text = "Sending request..."
                                    statusMessage.color = Style.secondaryTextColor
                                    networkManager.requestAdminUpdateGame(
                                        gameIdField.text,
                                        typeField.currentText,
                                        levelField.currentText,
                                        jsonField.text
                                    )
                                }
                            }
                        }
                    }
                }

                // Delete Game Section
                Rectangle {
                    Layout.fillWidth: true
                    height: deleteLayout.height + 40
                    color: Style.cardBackground
                    radius: Style.cornerRadius
                    
                    ColumnLayout {
                        id: deleteLayout
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 15

                        Text {
                            text: "Delete Game"
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.textColor
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            TextField { 
                                id: deleteIdField
                                placeholderText: "Game ID to delete"
                                Layout.fillWidth: true 
                            }
                            
                            Button {
                                text: "Delete"
                                enabled: deleteIdField.text.length > 0
                                background: Rectangle {
                                    color: style.enabled ? Style.errorColor : Style.secondaryColor
                                    property var style: parent
                                    radius: Style.cornerRadius
                                }
                                contentItem: Text {
                                    text: parent.text
                                    color: "white"
                                    font.bold: true
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter
                                }
                                onClicked: {
                                    statusMessage.text = "Sending request..."
                                    statusMessage.color = Style.secondaryTextColor
                                    networkManager.requestAdminDeleteGame(deleteIdField.text)
                                }
                            }
                        }
                    }
                }
                
                Text {
                    id: statusMessage
                    text: ""
                    font.pixelSize: Style.bodySize
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }
}
