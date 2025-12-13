import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "."

Page {
    background: Rectangle { color: Style.backgroundColor }

    ListModel { id: gameListModel }
    ListModel { id: itemsModel }

    Component.onCompleted: {
        networkManager.requestGameLevelList("sentence_match")
    }

    Connections {
        target: networkManager
        
        function onGameLevelListReceived(listData) {
             gameListModel.clear()
             if (listData.length === 0) return

             var parts = listData.split(";")
             
             for (var i = 0; i < parts.length; i++) {
                 if (parts[i].length > 0) {
                     var dtoParts = parts[i].split("|")
                     if (dtoParts.length >= 2) {
                         var id = dtoParts[0]
                         var level = dtoParts[1]
                         var status = dtoParts[2] 
                         
                         gameListModel.append({ 
                             gameId: id,
                             gameLevel: level, 
                             status: status
                         })
                     }
                 }
             }
        }
        
        function onGameDataReceived(data) {
             // Parse GameContentDTO: id|type|level|json
             var parts = data.split("|")
             if (parts.length >= 4) {
                 var id = parts[0]
                 var type = parts[1]
                 var level = parts[2]
                 // JSON can contain pipe, so join the rest
                 var json = parts.slice(3).join("|")
                 
                 gameIdField.text = id
                 
                 // Update Type Combo
                 for(var i=0; i<typeField.model.length; i++) {
                     if(typeField.model[i] === type) typeField.currentIndex = i
                 }

                 // Update Level Combo
                 var levelLower = level.toLowerCase()
                 for(var j=0; j<levelField.model.length; j++) {
                     if(levelField.model[j].toLowerCase() === levelLower) levelField.currentIndex = j
                 }
                 
                 jsonField.text = json
                 
                 // Parse JSON into itemsModel
                 itemsModel.clear()
                 try {
                     var arr = JSON.parse(json)
                     if (Array.isArray(arr)) {
                         for(var k=0; k<arr.length; k++) {
                             itemsModel.append({ "question": arr[k].question || "", "answer": arr[k].answer || "" })
                         }
                     }
                 } catch(e) {
                     console.log("Error parsing JSON for items: " + e)
                 }
             }
        }

        function onGameCreateSuccess(message) {
            statusMessage.text = "Game created successfully!"
            statusMessage.color = Style.successColor
            // Refresh list
            networkManager.requestGameLevelList(filterTypeCombo.currentText)
            // Clear inputs
            clearForm()
        }

        function onGameCreateFailure(message) {
            statusMessage.text = "Error creating game: " + message
            statusMessage.color = Style.errorColor
        }
        
        function onGameUpdateSuccess(message) {
            statusMessage.text = "Game updated successfully!"
            statusMessage.color = Style.successColor
            networkManager.requestGameLevelList(filterTypeCombo.currentText)
            clearForm()
        }

        function onGameUpdateFailure(message) {
            statusMessage.text = "Error updating game: " + message
            statusMessage.color = Style.errorColor
        }

        function onGameDeleteSuccess(message) {
            statusMessage.text = "Game deleted successfully!"
            statusMessage.color = Style.successColor
            // Refresh list
            networkManager.requestGameLevelList(filterTypeCombo.currentText)
            // If the deleted game was being edited, clear form
            if (message.indexOf(gameIdField.text) !== -1) {
                clearForm()
            }
        }

        function onGameDeleteFailure(message) {
            statusMessage.text = "Error deleting game: " + message
            statusMessage.color = Style.errorColor
        }
    }

    function clearForm() {
        gameIdField.text = ""
        itemsModel.clear()
        jsonField.text = ""
        inputQ.text = ""
        inputA.text = ""
    }

    function updateJson() {
        var arr = []
        for(var i=0; i<itemsModel.count; i++) {
             var item = itemsModel.get(i)
             arr.push({ "question": item.question, "answer": item.answer })
        }
        jsonField.text = JSON.stringify(arr, null, 2)
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Style.margin
        spacing: Style.margin

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
        
        Text {
            id: statusMessage
            text: ""
            font.pixelSize: Style.bodySize
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        // Main 2-Column Layout
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: Style.margin

            // LEFT COLUMN: Game List
            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: parent.width * 0.4
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Style.margin
                    spacing: 15

                    Text {
                        text: "Existing Games"
                        font.pixelSize: Style.subHeadingSize
                        font.bold: true
                        color: Style.textColor
                    }

                    // Filter
                    RowLayout {
                        Layout.fillWidth: true
                        Text { text: "Type:"; color: Style.secondaryTextColor }
                        ComboBox {
                            id: filterTypeCombo
                            Layout.fillWidth: true
                            model: ["sentence_match", "word_match", "image_match"]
                            onCurrentTextChanged: {
                                networkManager.requestGameLevelList(currentText)
                            }
                        }
                    }
                    
                    Rectangle { height: 1; Layout.fillWidth: true; color: "#f0f0f0" }

                    // List
                    ListView {
                        id: gameList
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: gameListModel
                        spacing: 8

                        delegate: Rectangle {
                            width: gameList.width
                            height: 60
                            color: "#f8f9fa"
                            radius: Style.cornerRadius
                            border.color: gameIdField.text === model.gameId ? Style.primaryColor : "#e0e0e0"
                            border.width: gameIdField.text === model.gameId ? 2 : 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 10
                                spacing: 10
                                
                                Rectangle {
                                    width: 80
                                    height: 24
                                    radius: 12
                                    color: {
                                        var lvl = model.gameLevel.toLowerCase()
                                        if (lvl === "beginner") return Style.successColor
                                        if (lvl === "intermediate") return Style.primaryColor
                                        return Style.errorColor
                                    }
                                    Text {
                                        anchors.centerIn: parent
                                        text: model.gameLevel
                                        color: "white"
                                        font.pixelSize: 10
                                        font.bold: true
                                    }
                                }

                                Text {
                                    text: "ID: " + model.gameId
                                    font.bold: true
                                    color: Style.textColor
                                    Layout.fillWidth: true
                                }
                                
                                Button {
                                    text: "Edit"
                                    flat: true
                                    onClicked: {
                                        statusMessage.text = "Loading..."
                                        statusMessage.color = Style.secondaryTextColor
                                        networkManager.requestGameData(model.gameId)
                                    }
                                }
                                
                                Button {
                                    text: "×"
                                    flat: true
                                    onClicked: {
                                        statusMessage.text = "Deleting..."
                                        statusMessage.color = Style.secondaryTextColor
                                        networkManager.requestAdminDeleteGame(model.gameId)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // RIGHT COLUMN: Editor
            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: Style.cardBackground
                radius: Style.cornerRadius
                border.color: "#e0e0e0"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: Style.margin
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: gameIdField.text === "" ? "Create New Game" : "Edit Game " + gameIdField.text
                            font.pixelSize: Style.subHeadingSize
                            font.bold: true
                            color: Style.primaryColor
                            Layout.fillWidth: true
                        }
                        
                        Button {
                            text: "Clear / New"
                            onClicked: clearForm()
                        }
                    }
                    
                    Rectangle { height: 1; Layout.fillWidth: true; color: "#f0f0f0" }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        // Hidden ID field logic, just displayed in title
                        TextField { 
                            id: gameIdField 
                            visible: false 
                            readOnly: true
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            Text { text: "Type"; color: Style.secondaryTextColor; font.pixelSize: Style.smallSize }
                            ComboBox {
                                id: typeField
                                Layout.fillWidth: true
                                model: ["sentence_match", "word_match", "image_match"]
                            }
                        }
                        
                        ColumnLayout {
                            Layout.fillWidth: true
                            Text { text: "Level"; color: Style.secondaryTextColor; font.pixelSize: Style.smallSize }
                            ComboBox {
                                id: levelField
                                Layout.fillWidth: true
                                model: ["beginner", "intermediate", "advanced"]
                            }
                        }
                    }

                    // Dynamic Item Manager
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: "#ffffff"
                        
                        ColumnLayout {
                            anchors.fill: parent
                            spacing: 10
                            
                            Text { 
                                text: "Game Items"
                                font.bold: true
                                color: Style.textColor 
                            }
                            
                            // Input Row
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                
                                property string qLabel: {
                                    if (typeField.currentText === "image_match") return "Image URL"
                                    if (typeField.currentText === "sentence_match") return "Sentence Part 1"
                                    return "Word"
                                }
                                property string aLabel: {
                                    if (typeField.currentText === "image_match") return "Correct Word"
                                    if (typeField.currentText === "sentence_match") return "Sentence Part 2"
                                    return "Meaning/Match"
                                }

                                TextField {
                                    id: inputQ
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 2
                                    placeholderText: parent.qLabel
                                    font.pixelSize: Style.bodySize
                                    onAccepted: inputA.forceActiveFocus()
                                }
                                TextField {
                                    id: inputA
                                    Layout.fillWidth: true
                                    Layout.preferredWidth: 2
                                    placeholderText: parent.aLabel
                                    font.pixelSize: Style.bodySize
                                    onAccepted: addBtn.clicked() 
                                }
                                Button {
                                    id: addBtn
                                    text: "Add"
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
                                        if (inputQ.text !== "" && inputA.text !== "") {
                                            itemsModel.append({ "question": inputQ.text, "answer": inputA.text })
                                            inputQ.text = ""
                                            inputA.text = ""
                                            inputQ.forceActiveFocus()
                                            updateJson()
                                        }
                                    }
                                }
                            }
                            
                            // Items List
                            ListView {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                model: itemsModel
                                clip: true
                                spacing: 5
                                delegate: Rectangle {
                                    width: ListView.view ? ListView.view.width : 0
                                    height: 40
                                    color: "#f8f9fa"
                                    radius: 4
                                    border.color: "#e0e0e0"

                                    RowLayout {
                                        anchors.fill: parent
                                        anchors.margins: 5
                                        spacing: 10
                                        Text { 
                                            text: model.question
                                            font.pixelSize: Style.smallSize
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Text { text: "→"; color: Style.secondaryTextColor }
                                        Text { 
                                            text: model.answer 
                                            font.pixelSize: Style.smallSize
                                            Layout.fillWidth: true
                                            elide: Text.ElideRight
                                        }
                                        Button {
                                            text: "×"
                                            flat: true
                                            Layout.preferredWidth: 30
                                            onClicked: {
                                                itemsModel.remove(index)
                                                updateJson()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Raw JSON (Collapsable or Small)
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 5
                        Text { text: "Raw JSON (Generated)"; color: Style.secondaryTextColor; font.pixelSize: Style.smallSize }
                        TextArea {
                            id: jsonField
                            Layout.fillWidth: true
                            Layout.preferredHeight: 80
                            readOnly: true // Encourage using the builder
                            wrapMode: TextEdit.Wrap
                            background: Rectangle {
                                color: "#f1f5f9"
                                border.color: "#e0e0e0"
                                radius: Style.cornerRadius
                            }
                        }
                    }

                    // Actions
                    RowLayout {
                        Layout.alignment: Qt.AlignRight
                        spacing: 10

                        Button {
                            text: "Save Game"
                            background: Rectangle {
                                color: Style.primaryColor
                                radius: Style.cornerRadius
                            }
                            contentItem: Text {
                                text: gameIdField.text === "" ? "Create Game" : "Update Game"
                                color: "white"
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                verticalAlignment: Text.AlignVCenter
                            }
                            onClicked: {
                                statusMessage.text = "Saving..."
                                statusMessage.color = Style.secondaryTextColor
                                
                                if (gameIdField.text === "") {
                                    networkManager.requestAdminCreateGame(
                                        typeField.currentText,
                                        levelField.currentText,
                                        jsonField.text
                                    )
                                } else {
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
            }
        }
    }
}
